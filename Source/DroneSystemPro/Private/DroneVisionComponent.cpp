// Copyright Epic Games, Inc. All Rights Reserved.

#include "DroneVisionComponent.h"
#include "DroneBatteryComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "EngineUtils.h"

UDroneVisionComponent::UDroneVisionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	CurrentVisionMode = EDroneVisionMode::Normal;
	JammingIntensity = 0.0f;
	LastScanTime = 0.0f;
	ScanInterval = 0.5f; // Scan every 0.5 seconds
}

void UDroneVisionComponent::BeginPlay()
{
	Super::BeginPlay();

	ApplyVisionPostProcess();
}

void UDroneVisionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UDroneVisionComponent, CurrentVisionMode);
	DOREPLIFETIME(UDroneVisionComponent, ThermalDetections);
}

void UDroneVisionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	// Automatic thermal scanning in thermal mode
	if (CurrentVisionMode == EDroneVisionMode::Thermal)
	{
		float CurrentTime = GetWorld()->GetTimeSeconds();
		if (CurrentTime - LastScanTime >= ScanInterval)
		{
			PerformThermalDetection();
			LastScanTime = CurrentTime;
		}
	}
}

void UDroneVisionComponent::SetVisionMode(EDroneVisionMode NewMode)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		CurrentVisionMode = NewMode;
		Multicast_SetVisionMode(NewMode);
		NotifyBatteryComponent(NewMode);
	}
	else
	{
		Server_SetVisionMode(NewMode);
	}
}

void UDroneVisionComponent::CycleVisionMode()
{
	EDroneVisionMode NewMode;
	switch (CurrentVisionMode)
	{
	case EDroneVisionMode::Normal:
		NewMode = EDroneVisionMode::Night;
		break;
	case EDroneVisionMode::Night:
		NewMode = EDroneVisionMode::Thermal;
		break;
	case EDroneVisionMode::Thermal:
	default:
		NewMode = EDroneVisionMode::Normal;
		break;
	}

	SetVisionMode(NewMode);
}

void UDroneVisionComponent::PerformThermalScan()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		PerformThermalDetection();
	}
}

void UDroneVisionComponent::SetDroneConfig(UDroneConfig* NewConfig)
{
	DroneConfig = NewConfig;
}

float UDroneVisionComponent::GetEffectiveSensorRange() const
{
	if (!DroneConfig)
		return 3000.0f;

	float BaseRange = DroneConfig->SensorRange;

	// Apply jamming reduction
	float JammingReduction = JammingIntensity * 0.5f; // Up to 50% reduction
	BaseRange *= (1.0f - JammingReduction);

	return BaseRange;
}

void UDroneVisionComponent::SetJammingIntensity(float Intensity)
{
	JammingIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
}

void UDroneVisionComponent::Server_SetVisionMode_Implementation(EDroneVisionMode NewMode)
{
	CurrentVisionMode = NewMode;
	Multicast_SetVisionMode(NewMode);
	NotifyBatteryComponent(NewMode);
}

bool UDroneVisionComponent::Server_SetVisionMode_Validate(EDroneVisionMode NewMode)
{
	return true;
}

void UDroneVisionComponent::Multicast_SetVisionMode_Implementation(EDroneVisionMode NewMode)
{
	CurrentVisionMode = NewMode;
	ApplyVisionPostProcess();
	OnVisionModeChanged.Broadcast(NewMode);
}

void UDroneVisionComponent::OnRep_VisionMode()
{
	ApplyVisionPostProcess();
	OnVisionModeChanged.Broadcast(CurrentVisionMode);
}

void UDroneVisionComponent::PerformThermalDetection()
{
	if (!GetOwner() || !DroneConfig)
		return;

	ThermalDetections.Empty();

	float DetectionRange = FMath::Min(GetEffectiveSensorRange(), DroneConfig->ThermalDetectionRange);
	FVector OwnerLocation = GetOwner()->GetActorLocation();

	// Scan for actors in range
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor || Actor == GetOwner())
			continue;

		// Only detect characters and pawns for thermal signatures
		if (!Actor->IsA(ACharacter::StaticClass()) && !Actor->IsA(APawn::StaticClass()))
			continue;

		if (IsActorInRange(Actor, DetectionRange))
		{
			float HeatSignature = CalculateHeatSignature(Actor);
			if (HeatSignature > 0.1f) // Threshold for detection
			{
				FThermalDetection Detection(Actor, Actor->GetActorLocation(), HeatSignature);
				ThermalDetections.Add(Detection);
			}
		}
	}

	// Broadcast detection event
	OnThermalDetection.Broadcast(ThermalDetections);
}

float UDroneVisionComponent::CalculateHeatSignature(AActor* Actor) const
{
	if (!Actor)
		return 0.0f;

	// Base heat signature
	float Heat = 1.0f;

	// Characters have higher heat signatures
	if (Actor->IsA(ACharacter::StaticClass()))
	{
		Heat = 1.0f;
	}
	else if (Actor->IsA(APawn::StaticClass()))
	{
		Heat = 0.7f;
	}

	// Distance attenuation
	if (GetOwner())
	{
		float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
		float MaxRange = DroneConfig ? DroneConfig->ThermalDetectionRange : 2000.0f;
		float Attenuation = 1.0f - (Distance / MaxRange);
		Heat *= FMath::Clamp(Attenuation, 0.0f, 1.0f);
	}

	// Jamming reduces heat signature visibility
	Heat *= (1.0f - JammingIntensity * 0.3f);

	return FMath::Clamp(Heat, 0.0f, 1.0f);
}

bool UDroneVisionComponent::IsActorInRange(AActor* Actor, float Range) const
{
	if (!Actor || !GetOwner())
		return false;

	float DistanceSq = FVector::DistSquared(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
	return DistanceSq <= (Range * Range);
}

void UDroneVisionComponent::NotifyBatteryComponent(EDroneVisionMode Mode)
{
	if (!GetOwner())
		return;

	UDroneBatteryComponent* BatteryComp = GetOwner()->FindComponentByClass<UDroneBatteryComponent>();
	if (BatteryComp)
	{
		BatteryComp->SetVisionMode(Mode);
	}
}

void UDroneVisionComponent::ApplyVisionPostProcess()
{
	// This would apply post-process effects for night/thermal vision
	// In a full implementation, this would modify camera post-process settings
	// For now, this is a placeholder for blueprint implementation

	// Blueprint can listen to OnVisionModeChanged and apply visual effects
}
