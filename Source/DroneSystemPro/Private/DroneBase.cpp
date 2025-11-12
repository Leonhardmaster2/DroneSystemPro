// Copyright Epic Games, Inc. All Rights Reserved.

#include "DroneBase.h"
#include "DroneMovementComponent.h"
#include "DroneBatteryComponent.h"
#include "DroneVisionComponent.h"
#include "DroneMarkingComponent.h"
#include "DroneUtilityComponent.h"
#include "DroneReplicationComponent.h"
#include "DroneCameraEffectsComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

ADroneBase::ADroneBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bIsActive = true;
	LookUpValue = 0.0f;
	TurnValue = 0.0f;
	PendingMovementInput = FVector::ZeroVector;
	LastMovementInput = FVector::ZeroVector;
	ControlRotationInput = FVector::ZeroVector;

	// Create mesh component
	DroneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DroneMesh"));
	RootComponent = DroneMesh;
	DroneMesh->SetCollisionProfileName(TEXT("Pawn"));
	DroneMesh->SetSimulatePhysics(false);

	// Create camera arm
	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	CameraArm->SetupAttachment(RootComponent);
	CameraArm->TargetArmLength = 300.0f;
	CameraArm->bUsePawnControlRotation = false;
	CameraArm->bInheritPitch = true;
	CameraArm->bInheritYaw = true;
	CameraArm->bInheritRoll = false;

	// Create camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraArm);

	// Create drone components
	DroneMovement = CreateDefaultSubobject<UDroneMovementComponent>(TEXT("DroneMovement"));
	DroneBattery = CreateDefaultSubobject<UDroneBatteryComponent>(TEXT("DroneBattery"));
	DroneVision = CreateDefaultSubobject<UDroneVisionComponent>(TEXT("DroneVision"));
	DroneMarking = CreateDefaultSubobject<UDroneMarkingComponent>(TEXT("DroneMarking"));
	DroneUtility = CreateDefaultSubobject<UDroneUtilityComponent>(TEXT("DroneUtility"));
	DroneReplication = CreateDefaultSubobject<UDroneReplicationComponent>(TEXT("DroneReplication"));
	DroneCameraEffects = CreateDefaultSubobject<UDroneCameraEffectsComponent>(TEXT("DroneCameraEffects"));

	// Set default net settings
	NetCullDistanceSquared = 15000.0f * 15000.0f;
	NetUpdateFrequency = 20.0f;
	MinNetUpdateFrequency = 10.0f;
}

void ADroneBase::BeginPlay()
{
	Super::BeginPlay();

	// Apply configuration to all components
	if (DroneConfig)
	{
		if (DroneMovement)
			DroneMovement->SetDroneConfig(DroneConfig);
		if (DroneBattery)
			DroneBattery->SetDroneConfig(DroneConfig);
		if (DroneVision)
			DroneVision->SetDroneConfig(DroneConfig);
		if (DroneMarking)
			DroneMarking->SetDroneConfig(DroneConfig);
	}
}

void ADroneBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADroneBase, bIsActive);
}

void ADroneBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Set up autonomous proxy role for client prediction
	if (HasAuthority())
	{
		SetAutonomousProxy(true);
	}
}

void ADroneBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsActive && DroneMovement)
	{
		// Consume and apply movement input vector
		FVector MovementInput = ConsumeMovementInputVector();
		if (!MovementInput.IsNearlyZero())
		{
			DroneMovement->SetMovementInput(MovementInput);
		}

		// Apply control rotation input
		if (!ControlRotationInput.IsNearlyZero())
		{
			FVector2D LookInput(ControlRotationInput.Z, ControlRotationInput.Y); // Yaw, Pitch
			DroneMovement->SetLookInput(LookInput);
			ControlRotationInput = FVector::ZeroVector;
		}
	}

	// Update camera based on drone rotation
	if (CameraArm)
	{
		FRotator CurrentRot = CameraArm->GetRelativeRotation();
		CameraArm->SetRelativeRotation(FRotator(CurrentRot.Pitch, CurrentRot.Yaw, 0.0f));
	}
}

void ADroneBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (!PlayerInputComponent)
		return;

	// Movement
	PlayerInputComponent->BindAxis("MoveForward", this, &ADroneBase::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ADroneBase::MoveRight);
	PlayerInputComponent->BindAxis("MoveUp", this, &ADroneBase::MoveUp);

	// Look
	PlayerInputComponent->BindAxis("LookUp", this, &ADroneBase::LookUp);
	PlayerInputComponent->BindAxis("Turn", this, &ADroneBase::Turn);

	// Actions
	PlayerInputComponent->BindAction("ToggleSpeed", IE_Pressed, this, &ADroneBase::ToggleSpeedMode);
	PlayerInputComponent->BindAction("ToggleFlashlight", IE_Pressed, this, &ADroneBase::ToggleFlashlight);
	PlayerInputComponent->BindAction("ToggleVision", IE_Pressed, this, &ADroneBase::ToggleVisionMode);
	PlayerInputComponent->BindAction("Mark", IE_Pressed, this, &ADroneBase::MarkTarget);
}

void ADroneBase::SetDroneConfig(UDroneConfig* NewConfig)
{
	DroneConfig = NewConfig;

	// Apply to all components
	if (DroneMovement)
		DroneMovement->SetDroneConfig(NewConfig);
	if (DroneBattery)
		DroneBattery->SetDroneConfig(NewConfig);
	if (DroneVision)
		DroneVision->SetDroneConfig(NewConfig);
	if (DroneMarking)
		DroneMarking->SetDroneConfig(NewConfig);
}

void ADroneBase::SetActive(bool bNewActive)
{
	bIsActive = bNewActive;

	if (HasAuthority())
	{
		// Disable components when inactive
		if (DroneMovement)
			DroneMovement->SetComponentTickEnabled(bNewActive);
		if (DroneBattery && !bNewActive)
			DroneBattery->StopDrain();
	}
}

void ADroneBase::AddMovementInput(FVector WorldDirection, float ScaleValue, bool bForce)
{
	if (!bIsActive)
		return;

	// Accumulate movement input
	PendingMovementInput += WorldDirection * ScaleValue;
}

void ADroneBase::AddControllerYawInput(float Val)
{
	if (!bIsActive || Val == 0.0f)
		return;

	ControlRotationInput.Z += Val;
}

void ADroneBase::AddControllerPitchInput(float Val)
{
	if (!bIsActive || Val == 0.0f)
		return;

	ControlRotationInput.Y += Val;
}

void ADroneBase::AddControllerRollInput(float Val)
{
	if (!bIsActive || Val == 0.0f)
		return;

	ControlRotationInput.X += Val;
}

FVector ADroneBase::ConsumeMovementInputVector()
{
	LastMovementInput = PendingMovementInput;
	PendingMovementInput = FVector::ZeroVector;
	return LastMovementInput;
}

void ADroneBase::MoveForward(float Value)
{
	if (!bIsActive || !DroneMovement)
		return;

	FVector Input = DroneMovement->GetMovementInput();
	Input.X = Value;
	DroneMovement->SetMovementInput(Input);
}

void ADroneBase::MoveRight(float Value)
{
	if (!bIsActive || !DroneMovement)
		return;

	FVector Input = DroneMovement->GetMovementInput();
	Input.Y = Value;
	DroneMovement->SetMovementInput(Input);
}

void ADroneBase::MoveUp(float Value)
{
	if (!bIsActive || !DroneMovement)
		return;

	FVector Input = DroneMovement->GetMovementInput();
	Input.Z = Value;
	DroneMovement->SetMovementInput(Input);
}

void ADroneBase::LookUp(float Value)
{
	LookUpValue = Value;
	if (DroneMovement)
	{
		DroneMovement->SetLookInput(FVector2D(TurnValue, LookUpValue));
	}
}

void ADroneBase::Turn(float Value)
{
	TurnValue = Value;
	if (DroneMovement)
	{
		DroneMovement->SetLookInput(FVector2D(TurnValue, LookUpValue));
	}
}

void ADroneBase::ToggleSpeedMode()
{
	if (!bIsActive || !DroneMovement)
		return;

	EDroneSpeedMode CurrentMode = DroneMovement->GetSpeedMode();
	EDroneSpeedMode NewMode = (CurrentMode == EDroneSpeedMode::Low) ? EDroneSpeedMode::High : EDroneSpeedMode::Low;
	DroneMovement->SetSpeedMode(NewMode);
}

void ADroneBase::ToggleFlashlight()
{
	if (!bIsActive || !DroneUtility)
		return;

	DroneUtility->ToggleFlashlight();
}

void ADroneBase::ToggleVisionMode()
{
	if (!bIsActive || !DroneVision)
		return;

	DroneVision->CycleVisionMode();
}

void ADroneBase::MarkTarget()
{
	if (!bIsActive || !DroneMarking)
		return;

	DroneMarking->MarkTargetInCrosshair();
}
