// Copyright Epic Games, Inc. All Rights Reserved.

#include "DroneBatteryComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

UDroneBatteryComponent::UDroneBatteryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	BatteryLevel = 100.0f;
	bIsRecharging = false;
	bIsDraining = true;
	bFlashlightActive = false;
	CurrentVisionMode = EDroneVisionMode::Normal;
	bIsScanning = false;
	CurrentSpeedMode = EDroneSpeedMode::Low;
	bWasDepleted = false;
}

void UDroneBatteryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (DroneConfig)
	{
		BatteryLevel = DroneConfig->MaxBattery;
	}
}

void UDroneBatteryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UDroneBatteryComponent, BatteryLevel);
	DOREPLIFETIME(UDroneBatteryComponent, bIsRecharging);
}

void UDroneBatteryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	if (bIsRecharging)
	{
		// Recharge battery
		float RechargeRate = DroneConfig ? DroneConfig->BatteryRechargeRate : 5.0f;
		float OldLevel = BatteryLevel;
		BatteryLevel = FMath::Min(BatteryLevel + (RechargeRate * DeltaTime), GetMaxBattery());

		if (BatteryLevel != OldLevel)
		{
			OnBatteryChanged.Broadcast(BatteryLevel);
		}

		// Check if fully recharged
		if (BatteryLevel >= GetMaxBattery() && bWasDepleted)
		{
			bWasDepleted = false;
			OnBatteryRecharged.Broadcast();
		}
	}
	else if (bIsDraining)
	{
		// Drain battery
		CalculateAndApplyDrain(DeltaTime);
	}
}

float UDroneBatteryComponent::GetBatteryPercent() const
{
	float MaxBattery = DroneConfig ? DroneConfig->MaxBattery : 100.0f;
	return (MaxBattery > 0.0f) ? (BatteryLevel / MaxBattery) * 100.0f : 0.0f;
}

void UDroneBatteryComponent::SetBatteryLevel(float NewLevel)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	float OldLevel = BatteryLevel;
	BatteryLevel = FMath::Clamp(NewLevel, 0.0f, GetMaxBattery());

	if (BatteryLevel != OldLevel)
	{
		OnBatteryChanged.Broadcast(BatteryLevel);

		if (BatteryLevel <= 0.0f && !bWasDepleted)
		{
			bWasDepleted = true;
			OnBatteryDepleted.Broadcast();
		}
	}
}

void UDroneBatteryComponent::Recharge(float Amount)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	SetBatteryLevel(BatteryLevel + Amount);
}

void UDroneBatteryComponent::StartRecharging()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	bIsRecharging = true;
	bIsDraining = false;
}

void UDroneBatteryComponent::StopRecharging()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	bIsRecharging = false;
	bIsDraining = true;
}

void UDroneBatteryComponent::StartDrain()
{
	bIsDraining = true;
}

void UDroneBatteryComponent::StopDrain()
{
	bIsDraining = false;
}

void UDroneBatteryComponent::SetFlashlightActive(bool bActive)
{
	bFlashlightActive = bActive;
}

void UDroneBatteryComponent::SetVisionMode(EDroneVisionMode Mode)
{
	CurrentVisionMode = Mode;
}

void UDroneBatteryComponent::SetScanning(bool bActive)
{
	bIsScanning = bActive;
}

void UDroneBatteryComponent::SetSpeedMode(EDroneSpeedMode Mode)
{
	CurrentSpeedMode = Mode;
}

void UDroneBatteryComponent::SetDroneConfig(UDroneConfig* NewConfig)
{
	DroneConfig = NewConfig;

	if (DroneConfig && BatteryLevel > DroneConfig->MaxBattery)
	{
		BatteryLevel = DroneConfig->MaxBattery;
	}
}

float UDroneBatteryComponent::GetCurrentDrainRate() const
{
	return CalculateTotalDrainRate();
}

void UDroneBatteryComponent::OnRep_BatteryLevel()
{
	OnBatteryChanged.Broadcast(BatteryLevel);

	if (BatteryLevel <= 0.0f && !bWasDepleted)
	{
		bWasDepleted = true;
		OnBatteryDepleted.Broadcast();
	}
}

void UDroneBatteryComponent::CalculateAndApplyDrain(float DeltaTime)
{
	if (BatteryLevel <= 0.0f)
	{
		if (!bWasDepleted)
		{
			bWasDepleted = true;
			OnBatteryDepleted.Broadcast();
		}
		return;
	}

	float DrainRate = CalculateTotalDrainRate();
	float OldLevel = BatteryLevel;
	BatteryLevel = FMath::Max(0.0f, BatteryLevel - (DrainRate * DeltaTime));

	if (BatteryLevel != OldLevel)
	{
		OnBatteryChanged.Broadcast(BatteryLevel);
	}

	if (BatteryLevel <= 0.0f && !bWasDepleted)
	{
		bWasDepleted = true;
		OnBatteryDepleted.Broadcast();
	}
}

float UDroneBatteryComponent::CalculateTotalDrainRate() const
{
	if (!DroneConfig)
		return 1.0f;

	float TotalDrain = 0.0f;

	// Base drain
	TotalDrain += DroneConfig->BatteryDrainIdle;

	// Speed mode drain
	if (CurrentSpeedMode == EDroneSpeedMode::High)
	{
		TotalDrain += DroneConfig->BatteryDrainHighSpeed;
	}
	else
	{
		TotalDrain += DroneConfig->BatteryDrainLowSpeed;
	}

	// Flashlight drain
	if (bFlashlightActive)
	{
		TotalDrain += DroneConfig->BatteryDrainFlashlight;
	}

	// Vision mode drain
	switch (CurrentVisionMode)
	{
	case EDroneVisionMode::Night:
		TotalDrain += DroneConfig->BatteryDrainNightVision;
		break;
	case EDroneVisionMode::Thermal:
		TotalDrain += DroneConfig->BatteryDrainThermalVision;
		break;
	default:
		break;
	}

	// Scanning drain
	if (bIsScanning)
	{
		TotalDrain += DroneConfig->BatteryDrainScanning;
	}

	return TotalDrain;
}

float UDroneBatteryComponent::GetMaxBattery() const
{
	return DroneConfig ? DroneConfig->MaxBattery : 100.0f;
}
