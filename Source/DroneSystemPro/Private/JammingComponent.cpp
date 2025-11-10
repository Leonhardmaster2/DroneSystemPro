// Copyright Epic Games, Inc. All Rights Reserved.

#include "JammingComponent.h"
#include "DroneBase.h"
#include "DroneVisionComponent.h"
#include "DroneBatteryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "EngineUtils.h"

UJammingComponent::UJammingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	bJammingEnabled = false;
	JamStrength = 1.0f;
	JamRadius = 2000.0f;
	DefaultJamStrength = 1.0f;
	DefaultJamRadius = 2000.0f;
	BatteryDrainMultiplier = 1.5f;
	SensorRangeMultiplier = 0.5f;
}

void UJammingComponent::BeginPlay()
{
	Super::BeginPlay();

	JamStrength = DefaultJamStrength;
	JamRadius = DefaultJamRadius;
}

void UJammingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UJammingComponent, bJammingEnabled);
	DOREPLIFETIME(UJammingComponent, JamStrength);
	DOREPLIFETIME(UJammingComponent, JamRadius);
}

void UJammingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	if (bJammingEnabled)
	{
		ApplyJammingEffects();
	}
}

void UJammingComponent::SetJammingEnabled(bool bEnabled)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		bJammingEnabled = bEnabled;
	}
}

void UJammingComponent::SetJamStrength(float Strength)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		JamStrength = FMath::Clamp(Strength, 0.0f, 2.0f);
	}
}

void UJammingComponent::SetJamRadius(float Radius)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		JamRadius = FMath::Max(0.0f, Radius);
	}
}

float UJammingComponent::GetJammingIntensityAtLocation(FVector Location) const
{
	if (!bJammingEnabled || !GetOwner())
		return 0.0f;

	float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Location);

	if (Distance > JamRadius)
		return 0.0f;

	// Intensity falls off with distance
	float Intensity = JamStrength * (1.0f - (Distance / JamRadius));
	return FMath::Clamp(Intensity, 0.0f, 1.0f);
}

TArray<AActor*> UJammingComponent::GetAffectedDrones() const
{
	TArray<AActor*> Result;

	if (!bJammingEnabled || !GetWorld())
		return Result;

	FVector JammerLocation = GetOwner()->GetActorLocation();

	for (TActorIterator<ADroneBase> It(GetWorld()); It; ++It)
	{
		ADroneBase* Drone = *It;
		if (!Drone)
			continue;

		float Distance = FVector::Dist(JammerLocation, Drone->GetActorLocation());
		if (Distance <= JamRadius)
		{
			Result.Add(Drone);
		}
	}

	return Result;
}

void UJammingComponent::OnRep_JammingEnabled()
{
	// Update visual effects when jamming state changes
	// Blueprint can handle visual effects
}

void UJammingComponent::ApplyJammingEffects()
{
	if (!GetWorld())
		return;

	UpdateAffectedDrones();
}

void UJammingComponent::UpdateAffectedDrones()
{
	if (!GetOwner())
		return;

	FVector JammerLocation = GetOwner()->GetActorLocation();

	for (TActorIterator<ADroneBase> It(GetWorld()); It; ++It)
	{
		ADroneBase* Drone = *It;
		if (!Drone)
			continue;

		float Distance = FVector::Dist(JammerLocation, Drone->GetActorLocation());

		if (Distance <= JamRadius)
		{
			float Intensity = GetJammingIntensityAtLocation(Drone->GetActorLocation());

			// Apply jamming to vision component
			UDroneVisionComponent* VisionComp = Drone->GetDroneVision();
			if (VisionComp)
			{
				VisionComp->SetJammingIntensity(Intensity);
			}

			// Note: Battery drain multiplier would be applied in battery component
			// based on environmental factors
		}
		else
		{
			// Clear jamming effects
			UDroneVisionComponent* VisionComp = Drone->GetDroneVision();
			if (VisionComp)
			{
				VisionComp->SetJammingIntensity(0.0f);
			}
		}
	}
}
