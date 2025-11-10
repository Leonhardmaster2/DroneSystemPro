// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DroneDockingComponent.generated.h"

class ADroneBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDroneDocked, ADroneBase*, Drone);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDroneUndocked, ADroneBase*, Drone);

/**
 * Docking station component for drone recharging and auto-recall
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DRONESYSTEMPRO_API UDroneDockingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDroneDockingComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Docking control
	UFUNCTION(BlueprintCallable, Category = "Docking")
	bool DockDrone(ADroneBase* Drone);

	UFUNCTION(BlueprintCallable, Category = "Docking")
	void UndockDrone();

	UFUNCTION(BlueprintPure, Category = "Docking")
	bool IsDroneD ocked() const { return DockedDrone != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Docking")
	ADroneBase* GetDockedDrone() const { return DockedDrone; }

	// Auto-recall
	UFUNCTION(BlueprintCallable, Category = "Docking")
	void RecallDrone(ADroneBase* Drone);

	UFUNCTION(BlueprintCallable, Category = "Docking")
	void SetAutoRecallOnLowBattery(bool bEnabled) { bAutoRecallOnLowBattery = bEnabled; }

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Docking")
	void SetDockingRange(float Range) { DockingRange = Range; }

	UFUNCTION(BlueprintPure, Category = "Docking")
	float GetDockingRange() const { return DockingRange; }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Docking")
	FOnDroneDocked OnDroneDocked;

	UPROPERTY(BlueprintAssignable, Category = "Docking")
	FOnDroneUndocked OnDroneUndocked;

protected:
	// Network RPCs
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_DockDrone(ADroneBase* Drone);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_UndockDrone();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DroneDocked(ADroneBase* Drone);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DroneUndocked(ADroneBase* Drone);

	// Replication
	UPROPERTY(Replicated)
	ADroneBase* DockedDrone;

	// Configuration
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	float DockingRange;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	float LowBatteryThreshold;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	bool bAutoRecallOnLowBattery;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	FVector DockingOffset;

private:
	void ProcessRecharging(float DeltaTime);
	bool IsInDockingRange(ADroneBase* Drone) const;
	void CheckAutoRecall();
};
