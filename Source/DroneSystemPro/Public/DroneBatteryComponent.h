// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DroneTypes.h"
#include "DroneBatteryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBatteryChanged, float, NewBatteryLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBatteryDepleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBatteryRecharged);

/**
 * Manages drone battery with drain rates per feature
 * Supports auto-recharge at docking stations
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DRONESYSTEMPRO_API UDroneBatteryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDroneBatteryComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Battery management
	UFUNCTION(BlueprintPure, Category = "Battery")
	float GetBatteryLevel() const { return BatteryLevel; }

	UFUNCTION(BlueprintPure, Category = "Battery")
	float GetBatteryPercent() const;

	UFUNCTION(BlueprintPure, Category = "Battery")
	bool IsDepleted() const { return BatteryLevel <= 0.0f; }

	UFUNCTION(BlueprintCallable, Category = "Battery")
	void SetBatteryLevel(float NewLevel);

	UFUNCTION(BlueprintCallable, Category = "Battery")
	void Recharge(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Battery")
	void StartRecharging();

	UFUNCTION(BlueprintCallable, Category = "Battery")
	void StopRecharging();

	UFUNCTION(BlueprintPure, Category = "Battery")
	bool IsRecharging() const { return bIsRecharging; }

	// Drain control
	UFUNCTION(BlueprintCallable, Category = "Battery")
	void StartDrain();

	UFUNCTION(BlueprintCallable, Category = "Battery")
	void StopDrain();

	// Feature activation (affects drain rate)
	UFUNCTION(BlueprintCallable, Category = "Battery")
	void SetFlashlightActive(bool bActive);

	UFUNCTION(BlueprintCallable, Category = "Battery")
	void SetVisionMode(EDroneVisionMode Mode);

	UFUNCTION(BlueprintCallable, Category = "Battery")
	void SetScanning(bool bActive);

	UFUNCTION(BlueprintCallable, Category = "Battery")
	void SetSpeedMode(EDroneSpeedMode Mode);

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Battery")
	void SetDroneConfig(UDroneConfig* NewConfig);

	UFUNCTION(BlueprintPure, Category = "Battery")
	float GetCurrentDrainRate() const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Battery")
	FOnBatteryChanged OnBatteryChanged;

	UPROPERTY(BlueprintAssignable, Category = "Battery")
	FOnBatteryDepleted OnBatteryDepleted;

	UPROPERTY(BlueprintAssignable, Category = "Battery")
	FOnBatteryRecharged OnBatteryRecharged;

protected:
	// Replication
	UPROPERTY(ReplicatedUsing=OnRep_BatteryLevel)
	float BatteryLevel;

	UFUNCTION()
	void OnRep_BatteryLevel();

	UPROPERTY(Replicated)
	bool bIsRecharging;

	// Configuration
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	UDroneConfig* DroneConfig;

	// State
	UPROPERTY()
	bool bIsDraining;

	UPROPERTY()
	bool bFlashlightActive;

	UPROPERTY()
	EDroneVisionMode CurrentVisionMode;

	UPROPERTY()
	bool bIsScanning;

	UPROPERTY()
	EDroneSpeedMode CurrentSpeedMode;

	UPROPERTY()
	bool bWasDepleted;

private:
	void CalculateAndApplyDrain(float DeltaTime);
	float CalculateTotalDrainRate() const;
};
