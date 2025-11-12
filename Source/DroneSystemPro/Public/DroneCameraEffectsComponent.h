// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraShakeBase.h"
#include "DroneCameraEffectsComponent.generated.h"

class UCameraShakeBase;
class APlayerCameraManager;

/**
 * Handles camera effects for drone including shake, FOV changes, and screen effects
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DRONESYSTEMPRO_API UDroneCameraEffectsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDroneCameraEffectsComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// Camera Shake
	UFUNCTION(BlueprintCallable, Category = "Camera Effects")
	void PlayCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass, float Scale = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Camera Effects")
	void PlayMovementShake();

	UFUNCTION(BlueprintCallable, Category = "Camera Effects")
	void PlayCollisionShake(float Intensity = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Camera Effects")
	void PlayLandingShake();

	UFUNCTION(BlueprintCallable, Category = "Camera Effects")
	void PlayBoostShake();

	// FOV Effects
	UFUNCTION(BlueprintCallable, Category = "Camera Effects")
	void SetFOVOverride(float NewFOV, float BlendTime = 0.3f);

	UFUNCTION(BlueprintCallable, Category = "Camera Effects")
	void ClearFOVOverride(float BlendTime = 0.3f);

	UFUNCTION(BlueprintCallable, Category = "Camera Effects")
	void ApplySpeedFOV(float Speed, float MaxSpeed);

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Camera Effects")
	void SetShakeIntensityMultiplier(float Multiplier) { ShakeIntensityMultiplier = Multiplier; }

	UFUNCTION(BlueprintPure, Category = "Camera Effects")
	float GetShakeIntensityMultiplier() const { return ShakeIntensityMultiplier; }

protected:
	// Camera Shake Classes
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera Shake")
	TSubclassOf<UCameraShakeBase> MovementShakeClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera Shake")
	TSubclassOf<UCameraShakeBase> CollisionShakeClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera Shake")
	TSubclassOf<UCameraShakeBase> LandingShakeClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera Shake")
	TSubclassOf<UCameraShakeBase> BoostShakeClass;

	// Configuration
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	float ShakeIntensityMultiplier;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	float BaseFOV;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	float SpeedFOVMultiplier;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	float MaxSpeedFOV;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	bool bEnableMovementShake;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	float MovementShakeThreshold;

	// State
	UPROPERTY()
	float CurrentFOV;

	UPROPERTY()
	float TargetFOV;

	UPROPERTY()
	float FOVBlendSpeed;

	UPROPERTY()
	float LastVelocityMagnitude;

private:
	APlayerCameraManager* GetCameraManager() const;
	void UpdateFOV(float DeltaTime);
	void UpdateMovementShake(float DeltaTime);
};
