// Copyright Epic Games, Inc. All Rights Reserved.

#include "DroneCameraEffectsComponent.h"
#include "DroneBase.h"
#include "DroneMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UDroneCameraEffectsComponent::UDroneCameraEffectsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	ShakeIntensityMultiplier = 1.0f;
	BaseFOV = 90.0f;
	SpeedFOVMultiplier = 0.015f;
	MaxSpeedFOV = 110.0f;
	bEnableMovementShake = true;
	MovementShakeThreshold = 500.0f;

	CurrentFOV = BaseFOV;
	TargetFOV = BaseFOV;
	FOVBlendSpeed = 3.0f;
	LastVelocityMagnitude = 0.0f;
}

void UDroneCameraEffectsComponent::BeginPlay()
{
	Super::BeginPlay();

	// Set initial FOV
	if (GetOwner())
	{
		UCameraComponent* Camera = GetOwner()->FindComponentByClass<UCameraComponent>();
		if (Camera)
		{
			CurrentFOV = Camera->FieldOfView;
			TargetFOV = CurrentFOV;
			BaseFOV = CurrentFOV;
		}
	}
}

void UDroneCameraEffectsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateFOV(DeltaTime);

	if (bEnableMovementShake)
	{
		UpdateMovementShake(DeltaTime);
	}
}

void UDroneCameraEffectsComponent::PlayCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass, float Scale)
{
	if (!ShakeClass)
		return;

	APlayerCameraManager* CameraManager = GetCameraManager();
	if (CameraManager)
	{
		CameraManager->StartCameraShake(ShakeClass, Scale * ShakeIntensityMultiplier);
	}
}

void UDroneCameraEffectsComponent::PlayMovementShake()
{
	if (MovementShakeClass)
	{
		PlayCameraShake(MovementShakeClass, 0.3f);
	}
}

void UDroneCameraEffectsComponent::PlayCollisionShake(float Intensity)
{
	if (CollisionShakeClass)
	{
		PlayCameraShake(CollisionShakeClass, Intensity);
	}
}

void UDroneCameraEffectsComponent::PlayLandingShake()
{
	if (LandingShakeClass)
	{
		PlayCameraShake(LandingShakeClass, 1.0f);
	}
}

void UDroneCameraEffectsComponent::PlayBoostShake()
{
	if (BoostShakeClass)
	{
		PlayCameraShake(BoostShakeClass, 0.5f);
	}
}

void UDroneCameraEffectsComponent::SetFOVOverride(float NewFOV, float BlendTime)
{
	TargetFOV = FMath::Clamp(NewFOV, 60.0f, 120.0f);
	FOVBlendSpeed = (BlendTime > 0.0f) ? (1.0f / BlendTime) : 10.0f;
}

void UDroneCameraEffectsComponent::ClearFOVOverride(float BlendTime)
{
	TargetFOV = BaseFOV;
	FOVBlendSpeed = (BlendTime > 0.0f) ? (1.0f / BlendTime) : 10.0f;
}

void UDroneCameraEffectsComponent::ApplySpeedFOV(float Speed, float MaxSpeed)
{
	if (MaxSpeed <= 0.0f)
		return;

	// Calculate FOV based on speed
	float SpeedRatio = FMath::Clamp(Speed / MaxSpeed, 0.0f, 1.0f);
	float SpeedFOV = BaseFOV + (SpeedRatio * (MaxSpeedFOV - BaseFOV));

	TargetFOV = SpeedFOV;
	FOVBlendSpeed = 5.0f; // Fast blend for speed changes
}

APlayerCameraManager* UDroneCameraEffectsComponent::GetCameraManager() const
{
	if (!GetOwner())
		return nullptr;

	APlayerController* PC = Cast<APlayerController>(GetOwner()->GetInstigatorController());
	if (!PC)
		return nullptr;

	return PC->PlayerCameraManager;
}

void UDroneCameraEffectsComponent::UpdateFOV(float DeltaTime)
{
	if (!GetOwner())
		return;

	// Blend current FOV to target
	if (!FMath::IsNearlyEqual(CurrentFOV, TargetFOV, 0.1f))
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, FOVBlendSpeed);

		// Apply to camera
		UCameraComponent* Camera = GetOwner()->FindComponentByClass<UCameraComponent>();
		if (Camera)
		{
			Camera->SetFieldOfView(CurrentFOV);
		}
	}

	// Auto-apply speed FOV
	ADroneBase* Drone = Cast<ADroneBase>(GetOwner());
	if (Drone)
	{
		UDroneMovementComponent* MovementComp = Drone->GetDroneMovement();
		if (MovementComp)
		{
			float Speed = MovementComp->GetCurrentSpeed();
			float MaxSpeed = MovementComp->GetDroneConfig() ?
				MovementComp->GetDroneConfig()->MaxSpeedHigh : 1200.0f;

			ApplySpeedFOV(Speed, MaxSpeed);
		}
	}
}

void UDroneCameraEffectsComponent::UpdateMovementShake(float DeltaTime)
{
	if (!GetOwner())
		return;

	ADroneBase* Drone = Cast<ADroneBase>(GetOwner());
	if (!Drone)
		return;

	UDroneMovementComponent* MovementComp = Drone->GetDroneMovement();
	if (!MovementComp)
		return;

	// Get current velocity
	float CurrentSpeed = MovementComp->GetCurrentSpeed();

	// Check for acceleration-based shake
	float AccelerationMagnitude = FMath::Abs(CurrentSpeed - LastVelocityMagnitude) / DeltaTime;

	if (AccelerationMagnitude > MovementShakeThreshold)
	{
		// Play subtle shake on sudden acceleration
		float ShakeScale = FMath::Clamp(AccelerationMagnitude / 2000.0f, 0.1f, 0.5f);
		PlayMovementShake();
	}

	LastVelocityMagnitude = CurrentSpeed;
}
