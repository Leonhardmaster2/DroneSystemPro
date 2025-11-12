// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DroneTypes.h"
#include "DroneHUDWidget.generated.h"

class ADroneBase;
class UDroneBatteryComponent;
class UDroneVisionComponent;
class UDroneMarkingComponent;
class UDroneUtilityComponent;

/**
 * HUD Widget for displaying drone status and marked targets
 * Shows battery, speed, altitude, compass, vision mode, and marked enemy indicators
 */
UCLASS(Blueprintable)
class DRONESYSTEMPRO_API UDroneHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UDroneHUDWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	// Setup
	UFUNCTION(BlueprintCallable, Category = "Drone HUD")
	void SetDrone(ADroneBase* InDrone);

	UFUNCTION(BlueprintPure, Category = "Drone HUD")
	ADroneBase* GetDrone() const { return OwningDrone; }

	// Battery Info
	UFUNCTION(BlueprintPure, Category = "Drone HUD|Battery")
	float GetBatteryPercent() const;

	UFUNCTION(BlueprintPure, Category = "Drone HUD|Battery")
	FLinearColor GetBatteryColor() const;

	UFUNCTION(BlueprintPure, Category = "Drone HUD|Battery")
	bool IsBatteryLow() const;

	UFUNCTION(BlueprintPure, Category = "Drone HUD|Battery")
	bool IsBatteryCritical() const;

	// Movement Info
	UFUNCTION(BlueprintPure, Category = "Drone HUD|Movement")
	float GetSpeed() const;

	UFUNCTION(BlueprintPure, Category = "Drone HUD|Movement")
	float GetAltitude() const;

	UFUNCTION(BlueprintPure, Category = "Drone HUD|Movement")
	float GetCompassHeading() const;

	UFUNCTION(BlueprintPure, Category = "Drone HUD|Movement")
	FString GetSpeedText() const;

	UFUNCTION(BlueprintPure, Category = "Drone HUD|Movement")
	FString GetAltitudeText() const;

	UFUNCTION(BlueprintPure, Category = "Drone HUD|Movement")
	EDroneSpeedMode GetSpeedMode() const;

	// Vision Info
	UFUNCTION(BlueprintPure, Category = "Drone HUD|Vision")
	EDroneVisionMode GetVisionMode() const;

	UFUNCTION(BlueprintPure, Category = "Drone HUD|Vision")
	FString GetVisionModeText() const;

	UFUNCTION(BlueprintPure, Category = "Drone HUD|Vision")
	FLinearColor GetVisionModeColor() const;

	// Marked Targets
	UFUNCTION(BlueprintPure, Category = "Drone HUD|Marking")
	TArray<AActor*> GetMarkedTargets() const;

	UFUNCTION(BlueprintPure, Category = "Drone HUD|Marking")
	int32 GetMarkedTargetCount() const;

	// Thermal Detections
	UFUNCTION(BlueprintPure, Category = "Drone HUD|Thermal")
	TArray<FThermalDetection> GetThermalDetections() const;

	// Crosshair / Targeting
	UFUNCTION(BlueprintPure, Category = "Drone HUD|Targeting")
	bool HasTargetInCrosshair() const;

	UFUNCTION(BlueprintPure, Category = "Drone HUD|Targeting")
	AActor* GetTargetInCrosshair() const;

	UFUNCTION(BlueprintPure, Category = "Drone HUD|Targeting")
	float GetDistanceToTarget(AActor* Target) const;

	// Utility
	UFUNCTION(BlueprintPure, Category = "Drone HUD|Utility")
	bool IsFlashlightOn() const;

	// Screen Space Calculations
	UFUNCTION(BlueprintPure, Category = "Drone HUD|Screen")
	bool GetScreenPositionForActor(AActor* Actor, FVector2D& OutScreenPosition) const;

	UFUNCTION(BlueprintPure, Category = "Drone HUD|Screen")
	bool IsActorOnScreen(AActor* Actor) const;

protected:
	// Cached drone reference
	UPROPERTY(BlueprintReadOnly, Category = "Drone HUD")
	ADroneBase* OwningDrone;

	// Cached component references
	UPROPERTY()
	UDroneBatteryComponent* BatteryComponent;

	UPROPERTY()
	UDroneVisionComponent* VisionComponent;

	UPROPERTY()
	UDroneMarkingComponent* MarkingComponent;

	UPROPERTY()
	UDroneUtilityComponent* UtilityComponent;

	// Configuration
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	float LowBatteryThreshold;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	float CriticalBatteryThreshold;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	FLinearColor NormalBatteryColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	FLinearColor LowBatteryColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	FLinearColor CriticalBatteryColor;

private:
	void CacheComponents();
	FString FormatDistance(float Distance) const;
};
