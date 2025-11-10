// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DroneTypes.h"
#include "DroneVisionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVisionModeChanged, EDroneVisionMode, NewMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThermalDetection, const TArray<FThermalDetection>&, Detections);

/**
 * Manages vision modes (Normal, Night, Thermal)
 * Handles thermal detection with low-bandwidth heatmap transmission
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DRONESYSTEMPRO_API UDroneVisionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDroneVisionComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Vision mode control
	UFUNCTION(BlueprintCallable, Category = "Vision")
	void SetVisionMode(EDroneVisionMode NewMode);

	UFUNCTION(BlueprintCallable, Category = "Vision")
	void CycleVisionMode();

	UFUNCTION(BlueprintPure, Category = "Vision")
	EDroneVisionMode GetVisionMode() const { return CurrentVisionMode; }

	// Thermal detection
	UFUNCTION(BlueprintPure, Category = "Vision")
	TArray<FThermalDetection> GetThermalDetections() const { return ThermalDetections; }

	UFUNCTION(BlueprintCallable, Category = "Vision")
	void PerformThermalScan();

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Vision")
	void SetDroneConfig(UDroneConfig* NewConfig);

	UFUNCTION(BlueprintPure, Category = "Vision")
	float GetEffectiveSensorRange() const;

	// Environmental factors
	UFUNCTION(BlueprintCallable, Category = "Vision")
	void SetJammingIntensity(float Intensity);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Vision")
	FOnVisionModeChanged OnVisionModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Vision")
	FOnThermalDetection OnThermalDetection;

protected:
	// Network RPCs
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetVisionMode(EDroneVisionMode NewMode);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetVisionMode(EDroneVisionMode NewMode);

	// Replication
	UPROPERTY(ReplicatedUsing=OnRep_VisionMode)
	EDroneVisionMode CurrentVisionMode;

	UFUNCTION()
	void OnRep_VisionMode();

	UPROPERTY(Replicated)
	TArray<FThermalDetection> ThermalDetections;

	// Configuration
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	UDroneConfig* DroneConfig;

	// State
	UPROPERTY()
	float JammingIntensity;

	UPROPERTY()
	float LastScanTime;

	UPROPERTY()
	float ScanInterval;

private:
	void PerformThermalDetection();
	float CalculateHeatSignature(AActor* Actor) const;
	bool IsActorInRange(AActor* Actor, float Range) const;
	void NotifyBatteryComponent(EDroneVisionMode Mode);
	void ApplyVisionPostProcess();
};
