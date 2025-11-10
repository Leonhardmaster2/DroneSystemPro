// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DroneTypes.h"
#include "DroneMovementComponent.generated.h"

/**
 * Drone movement component with client prediction and server reconciliation
 * Handles physics-free smooth interpolation with high/low speed modes
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DRONESYSTEMPRO_API UDroneMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDroneMovementComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Input handling
	UFUNCTION(BlueprintCallable, Category = "Drone Movement")
	void SetMovementInput(FVector InInput);

	UFUNCTION(BlueprintCallable, Category = "Drone Movement")
	void SetLookInput(FVector2D InInput);

	UFUNCTION(BlueprintCallable, Category = "Drone Movement")
	void SetSpeedMode(EDroneSpeedMode NewMode);

	UFUNCTION(BlueprintPure, Category = "Drone Movement")
	EDroneSpeedMode GetSpeedMode() const { return SpeedMode; }

	UFUNCTION(BlueprintPure, Category = "Drone Movement")
	FVector GetVelocity() const { return Velocity; }

	UFUNCTION(BlueprintPure, Category = "Drone Movement")
	float GetCurrentSpeed() const { return Velocity.Size(); }

	UFUNCTION(BlueprintPure, Category = "Drone Movement")
	FVector GetMovementInput() const { return MovementInput; }

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Drone Movement")
	void SetDroneConfig(UDroneConfig* NewConfig);

	UFUNCTION(BlueprintPure, Category = "Drone Movement")
	UDroneConfig* GetDroneConfig() const { return DroneConfig; }

protected:
	// Movement simulation
	void SimulateMovement(float DeltaTime, const FDroneInputState& Input);
	void ApplyMovement(float DeltaTime);

	// Client prediction
	void ClientTick(float DeltaTime);
	void ServerTick(float DeltaTime);

	// Reconciliation
	void ReconcileWithServer(const FDroneMovementSnapshot& ServerSnapshot);

	// Network RPCs
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendInput(FDroneInputState Input);

	UFUNCTION(Client, Reliable)
	void Client_ReceiveCorrection(FDroneMovementSnapshot ServerSnapshot);

	// Replication
	UPROPERTY(ReplicatedUsing=OnRep_ServerSnapshot)
	FDroneMovementSnapshot ServerSnapshot;

	UFUNCTION()
	void OnRep_ServerSnapshot();

	// Configuration
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	UDroneConfig* DroneConfig;

	// State
	UPROPERTY(Replicated)
	EDroneSpeedMode SpeedMode;

	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	FVector MovementInput;

	UPROPERTY()
	FVector2D LookInput;

	// Client prediction state
	UPROPERTY()
	TArray<FDroneInputState> PendingInputs;

	UPROPERTY()
	TArray<FDroneMovementSnapshot> ClientSnapshots;

	UPROPERTY()
	uint32 NextInputID;

	UPROPERTY()
	float LastSendTime;

	UPROPERTY()
	float SendInterval;

	// Environmental factors
	UPROPERTY()
	float WindMultiplier;

	UPROPERTY()
	float JammingMultiplier;

private:
	// Helper functions
	float GetMaxSpeed() const;
	float GetAcceleration() const;
	FVector CalculateDesiredVelocity(const FVector& Input) const;
	void ClampVelocity();
	void UpdateRotation(float DeltaTime);
};
