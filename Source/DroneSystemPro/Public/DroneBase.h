// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "DroneTypes.h"
#include "DroneBase.generated.h"

class UDroneMovementComponent;
class UDroneBatteryComponent;
class UDroneVisionComponent;
class UDroneMarkingComponent;
class UDroneUtilityComponent;
class UDroneReplicationComponent;
class UCameraComponent;
class USpringArmComponent;
class UStaticMeshComponent;

/**
 * Base drone pawn with all components
 * Central actor that manages drone functionality
 */
UCLASS(Blueprintable)
class DRONESYSTEMPRO_API ADroneBase : public APawn
{
	GENERATED_BODY()

public:
	ADroneBase();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;

public:
	// Component accessors
	UFUNCTION(BlueprintPure, Category = "Drone")
	UDroneMovementComponent* GetDroneMovement() const { return DroneMovement; }

	UFUNCTION(BlueprintPure, Category = "Drone")
	UDroneBatteryComponent* GetDroneBattery() const { return DroneBattery; }

	UFUNCTION(BlueprintPure, Category = "Drone")
	UDroneVisionComponent* GetDroneVision() const { return DroneVision; }

	UFUNCTION(BlueprintPure, Category = "Drone")
	UDroneMarkingComponent* GetDroneMarking() const { return DroneMarking; }

	UFUNCTION(BlueprintPure, Category = "Drone")
	UDroneUtilityComponent* GetDroneUtility() const { return DroneUtility; }

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Drone")
	void SetDroneConfig(UDroneConfig* NewConfig);

	UFUNCTION(BlueprintPure, Category = "Drone")
	UDroneConfig* GetDroneConfig() const { return DroneConfig; }

	// State
	UFUNCTION(BlueprintPure, Category = "Drone")
	bool IsActive() const { return bIsActive; }

	UFUNCTION(BlueprintCallable, Category = "Drone")
	void SetActive(bool bNewActive);

protected:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DroneMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* CameraArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UDroneMovementComponent* DroneMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UDroneBatteryComponent* DroneBattery;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UDroneVisionComponent* DroneVision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UDroneMarkingComponent* DroneMarking;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UDroneUtilityComponent* DroneUtility;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UDroneReplicationComponent* DroneReplication;

	// Configuration
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	UDroneConfig* DroneConfig;

	// State
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "State")
	bool bIsActive;

	// Input callbacks
	void MoveForward(float Value);
	void MoveRight(float Value);
	void MoveUp(float Value);
	void LookUp(float Value);
	void Turn(float Value);
	void ToggleSpeedMode();
	void ToggleFlashlight();
	void ToggleVisionMode();
	void MarkTarget();

private:
	float LookUpValue;
	float TurnValue;
};
