// Copyright Epic Games, Inc. All Rights Reserved.

#include "DroneMovementComponent.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

UDroneMovementComponent::UDroneMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	SpeedMode = EDroneSpeedMode::Low;
	Velocity = FVector::ZeroVector;
	MovementInput = FVector::ZeroVector;
	LookInput = FVector2D::ZeroVector;
	NextInputID = 0;
	LastSendTime = 0.0f;
	SendInterval = 1.0f / 30.0f; // 30Hz send rate
	WindMultiplier = 1.0f;
	JammingMultiplier = 1.0f;
}

void UDroneMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// Load default config if not set
	if (!DroneConfig)
	{
		// Try to load a default config asset
		// In production, this would be set in editor or loaded from content
	}
}

void UDroneMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UDroneMovementComponent, ServerSnapshot);
	DOREPLIFETIME(UDroneMovementComponent, SpeedMode);
}

void UDroneMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner())
		return;

	// Different logic for server vs client
	if (GetOwner()->HasAuthority())
	{
		ServerTick(DeltaTime);
	}
	else if (GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
	{
		ClientTick(DeltaTime);
	}
	else
	{
		// Simulated proxy - interpolate to server snapshot
		if (ServerSnapshot.Timestamp > 0.0f)
		{
			FVector TargetLocation = ServerSnapshot.Location;
			FRotator TargetRotation = ServerSnapshot.Rotation;
			Velocity = ServerSnapshot.Velocity;

			FVector CurrentLocation = GetOwner()->GetActorLocation();
			FRotator CurrentRotation = GetOwner()->GetActorRotation();

			// Smooth interpolation
			float Alpha = FMath::Clamp(DeltaTime * 10.0f, 0.0f, 1.0f);
			FVector NewLocation = FMath::Lerp(CurrentLocation, TargetLocation, Alpha);
			FRotator NewRotation = FMath::Lerp(CurrentRotation, TargetRotation, Alpha);

			GetOwner()->SetActorLocation(NewLocation);
			GetOwner()->SetActorRotation(NewRotation);
		}
	}
}

void UDroneMovementComponent::SetMovementInput(FVector InInput)
{
	MovementInput = InInput.GetClampedToMaxSize(1.0f);
}

void UDroneMovementComponent::SetLookInput(FVector2D InInput)
{
	LookInput = InInput;
}

void UDroneMovementComponent::SetSpeedMode(EDroneSpeedMode NewMode)
{
	SpeedMode = NewMode;
}

void UDroneMovementComponent::SetDroneConfig(UDroneConfig* NewConfig)
{
	DroneConfig = NewConfig;
}

void UDroneMovementComponent::ClientTick(float DeltaTime)
{
	// Create input state
	FDroneInputState InputState;
	InputState.MovementInput = MovementInput;
	InputState.LookInput = LookInput;
	InputState.DeltaTime = DeltaTime;
	InputState.InputID = NextInputID++;
	InputState.Timestamp = GetWorld()->GetTimeSeconds();

	// Simulate movement locally
	SimulateMovement(DeltaTime, InputState);
	ApplyMovement(DeltaTime);

	// Store input and snapshot for reconciliation
	PendingInputs.Add(InputState);

	FDroneMovementSnapshot Snapshot(
		GetOwner()->GetActorLocation(),
		GetOwner()->GetActorRotation(),
		Velocity,
		InputState.Timestamp,
		InputState.InputID
	);
	ClientSnapshots.Add(Snapshot);

	// Clean old data (keep last 1 second)
	float CurrentTime = GetWorld()->GetTimeSeconds();
	PendingInputs.RemoveAll([CurrentTime](const FDroneInputState& Input) {
		return (CurrentTime - Input.Timestamp) > 1.0f;
	});
	ClientSnapshots.RemoveAll([CurrentTime](const FDroneMovementSnapshot& Snap) {
		return (CurrentTime - Snap.Timestamp) > 1.0f;
	});

	// Send input to server at fixed intervals
	if ((CurrentTime - LastSendTime) >= SendInterval)
	{
		Server_SendInput(InputState);
		LastSendTime = CurrentTime;
	}
}

void UDroneMovementComponent::ServerTick(float DeltaTime)
{
	// Server simulates with last received input
	FDroneInputState CurrentInput;
	CurrentInput.MovementInput = MovementInput;
	CurrentInput.LookInput = LookInput;
	CurrentInput.DeltaTime = DeltaTime;
	CurrentInput.Timestamp = GetWorld()->GetTimeSeconds();

	SimulateMovement(DeltaTime, CurrentInput);
	ApplyMovement(DeltaTime);

	// Update server snapshot
	ServerSnapshot.Location = GetOwner()->GetActorLocation();
	ServerSnapshot.Rotation = GetOwner()->GetActorRotation();
	ServerSnapshot.Velocity = Velocity;
	ServerSnapshot.Timestamp = GetWorld()->GetTimeSeconds();
}

void UDroneMovementComponent::SimulateMovement(float DeltaTime, const FDroneInputState& Input)
{
	if (!DroneConfig)
		return;

	// Calculate desired velocity based on input
	FVector DesiredVelocity = CalculateDesiredVelocity(Input.MovementInput);

	// Apply acceleration/deceleration
	float AccelRate = GetAcceleration();
	if (DesiredVelocity.IsNearlyZero())
	{
		// Decelerate
		AccelRate = DroneConfig->Deceleration;
	}

	// Lerp towards desired velocity
	float Alpha = FMath::Clamp(DeltaTime * AccelRate / GetMaxSpeed(), 0.0f, 1.0f);
	Velocity = FMath::Lerp(Velocity, DesiredVelocity, Alpha);

	// Apply environmental factors
	Velocity *= WindMultiplier * JammingMultiplier;

	ClampVelocity();
}

void UDroneMovementComponent::ApplyMovement(float DeltaTime)
{
	if (!GetOwner())
		return;

	// Move actor
	FVector NewLocation = GetOwner()->GetActorLocation() + (Velocity * DeltaTime);
	GetOwner()->SetActorLocation(NewLocation, true);

	// Update rotation based on velocity and look input
	UpdateRotation(DeltaTime);
}

void UDroneMovementComponent::UpdateRotation(float DeltaTime)
{
	if (!GetOwner() || !DroneConfig)
		return;

	FRotator CurrentRotation = GetOwner()->GetActorRotation();
	FRotator TargetRotation = CurrentRotation;

	// Yaw from look input
	if (!LookInput.IsNearlyZero())
	{
		TargetRotation.Yaw += LookInput.X * DroneConfig->TurnRate * DeltaTime;
	}

	// Pitch from look input
	if (!LookInput.IsNearlyZero())
	{
		TargetRotation.Pitch = FMath::Clamp(
			TargetRotation.Pitch + LookInput.Y * DroneConfig->TurnRate * DeltaTime,
			-DroneConfig->MaxPitchAngle,
			DroneConfig->MaxPitchAngle
		);
	}

	// Roll from movement (banking effect)
	if (!MovementInput.IsNearlyZero())
	{
		float RollAmount = MovementInput.Y * DroneConfig->MaxRollAngle;
		TargetRotation.Roll = FMath::FInterpTo(CurrentRotation.Roll, RollAmount, DeltaTime, 5.0f);
	}
	else
	{
		TargetRotation.Roll = FMath::FInterpTo(CurrentRotation.Roll, 0.0f, DeltaTime, 5.0f);
	}

	GetOwner()->SetActorRotation(TargetRotation);
}

void UDroneMovementComponent::Server_SendInput_Implementation(FDroneInputState Input)
{
	// Server validates and applies input
	MovementInput = Input.MovementInput.GetClampedToMaxSize(1.0f);
	LookInput = Input.LookInput;

	// Security: clamp values
	if (!DroneConfig)
		return;

	float MaxSpeed = GetMaxSpeed();
	if (Velocity.Size() > MaxSpeed * 1.5f) // Allow some tolerance
	{
		Velocity = Velocity.GetSafeNormal() * MaxSpeed;
	}

	// Store input ID for correction
	ServerSnapshot.InputID = Input.InputID;
}

bool UDroneMovementComponent::Server_SendInput_Validate(FDroneInputState Input)
{
	// Basic validation
	return Input.MovementInput.Size() <= 1.5f; // Allow some tolerance for floating point
}

void UDroneMovementComponent::Client_ReceiveCorrection_Implementation(FDroneMovementSnapshot ServerSnapshot)
{
	ReconcileWithServer(ServerSnapshot);
}

void UDroneMovementComponent::OnRep_ServerSnapshot()
{
	if (GetOwner() && GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
	{
		ReconcileWithServer(ServerSnapshot);
	}
}

void UDroneMovementComponent::ReconcileWithServer(const FDroneMovementSnapshot& InServerSnapshot)
{
	if (!GetOwner())
		return;

	// Find the client snapshot that corresponds to the server snapshot
	int32 SnapshotIndex = ClientSnapshots.IndexOfByPredicate([&](const FDroneMovementSnapshot& Snap) {
		return Snap.InputID == InServerSnapshot.InputID;
	});

	if (SnapshotIndex == INDEX_NONE)
		return;

	// Check if there's a significant error
	FVector PositionError = InServerSnapshot.Location - ClientSnapshots[SnapshotIndex].Location;
	float ErrorMagnitude = PositionError.Size();

	if (ErrorMagnitude > 50.0f) // Threshold for correction
	{
		// Snap to server position
		GetOwner()->SetActorLocation(InServerSnapshot.Location);
		GetOwner()->SetActorRotation(InServerSnapshot.Rotation);
		Velocity = InServerSnapshot.Velocity;

		// Remove old inputs
		int32 InputIndex = PendingInputs.IndexOfByPredicate([&](const FDroneInputState& Input) {
			return Input.InputID == InServerSnapshot.InputID;
		});

		if (InputIndex != INDEX_NONE)
		{
			PendingInputs.RemoveAt(0, InputIndex + 1);
			ClientSnapshots.RemoveAt(0, SnapshotIndex + 1);

			// Replay remaining inputs
			for (const FDroneInputState& Input : PendingInputs)
			{
				SimulateMovement(Input.DeltaTime, Input);
				ApplyMovement(Input.DeltaTime);
			}
		}
	}
}

float UDroneMovementComponent::GetMaxSpeed() const
{
	if (!DroneConfig)
		return 600.0f;

	return (SpeedMode == EDroneSpeedMode::High) ? DroneConfig->MaxSpeedHigh : DroneConfig->MaxSpeedLow;
}

float UDroneMovementComponent::GetAcceleration() const
{
	if (!DroneConfig)
		return 1000.0f;

	return DroneConfig->Acceleration;
}

FVector UDroneMovementComponent::CalculateDesiredVelocity(const FVector& Input) const
{
	if (!GetOwner())
		return FVector::ZeroVector;

	// Convert input to world space
	FRotator ActorRotation = GetOwner()->GetActorRotation();
	FVector Forward = FRotationMatrix(ActorRotation).GetScaledAxis(EAxis::X);
	FVector Right = FRotationMatrix(ActorRotation).GetScaledAxis(EAxis::Y);
	FVector Up = FVector::UpVector;

	FVector WorldInput = (Forward * Input.X) + (Right * Input.Y) + (Up * Input.Z);
	WorldInput = WorldInput.GetClampedToMaxSize(1.0f);

	return WorldInput * GetMaxSpeed();
}

void UDroneMovementComponent::ClampVelocity()
{
	float MaxSpeed = GetMaxSpeed();
	if (Velocity.Size() > MaxSpeed)
	{
		Velocity = Velocity.GetSafeNormal() * MaxSpeed;
	}
}
