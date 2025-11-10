// Copyright Epic Games, Inc. All Rights Reserved.

#include "DroneReplicationComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/NetDriver.h"

UDroneReplicationComponent::UDroneReplicationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(false); // This component doesn't need to replicate itself

	BasePriority = 1.0f;
	DistancePriorityScale = 1.0f;
	MaxRelevancyDistance = 15000.0f;
	BandwidthLimit = 0.0f; // 0 = unlimited
	CurrentBandwidthUsage = 0.0f;
	LastRelevancyUpdate = 0.0f;
}

void UDroneReplicationComponent::BeginPlay()
{
	Super::BeginPlay();

	OptimizeNetworkSettings();
	ApplyRelevancySettings();
}

void UDroneReplicationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	// Update relevancy periodically
	float CurrentTime = GetWorld()->GetTimeSeconds();
	float UpdateInterval = DroneConfig ? DroneConfig->RelevancyCheckInterval : 0.5f;

	if (CurrentTime - LastRelevancyUpdate >= UpdateInterval)
	{
		UpdateRelevancy();
		LastRelevancyUpdate = CurrentTime;
	}

	// Track bandwidth usage (simplified)
	CurrentBandwidthUsage = 0.0f; // Reset and recalculate
}

void UDroneReplicationComponent::UpdateRelevancy()
{
	if (!GetOwner())
		return;

	ApplyRelevancySettings();
}

bool UDroneReplicationComponent::IsRelevantTo(AActor* ViewingActor) const
{
	if (!ViewingActor || !GetOwner())
		return false;

	// Always relevant to owner
	if (ViewingActor == GetOwner()->GetOwner())
		return true;

	// Check distance
	float DistSq = FVector::DistSquared(GetOwner()->GetActorLocation(), ViewingActor->GetActorLocation());
	float MaxDistSq = MaxRelevancyDistance * MaxRelevancyDistance;

	return DistSq <= MaxDistSq;
}

float UDroneReplicationComponent::GetReplicationPriority(AActor* ViewingActor) const
{
	if (!ViewingActor || !GetOwner())
		return BasePriority;

	float Priority = BasePriority;

	// Distance-based priority
	float Distance = FVector::Dist(GetOwner()->GetActorLocation(), ViewingActor->GetActorLocation());
	float DistanceFactor = 1.0f - FMath::Clamp(Distance / MaxRelevancyDistance, 0.0f, 1.0f);
	Priority *= (1.0f + DistanceFactor * DistancePriorityScale);

	// Owner gets highest priority
	if (ViewingActor == GetOwner()->GetOwner())
	{
		Priority *= 10.0f;
	}

	return Priority;
}

void UDroneReplicationComponent::SetBandwidthLimit(float Limit)
{
	BandwidthLimit = FMath::Max(0.0f, Limit);
}

void UDroneReplicationComponent::SetDroneConfig(UDroneConfig* NewConfig)
{
	DroneConfig = NewConfig;
	OptimizeNetworkSettings();
	ApplyRelevancySettings();
}

void UDroneReplicationComponent::OptimizeNetworkSettings()
{
	if (!GetOwner() || !DroneConfig)
		return;

	// Apply network settings from config
	GetOwner()->NetUpdateFrequency = DroneConfig->ReplicationRate;
	GetOwner()->NetCullDistanceSquared = DroneConfig->NetCullDistance * DroneConfig->NetCullDistance;
}

void UDroneReplicationComponent::ApplyRelevancySettings()
{
	if (!GetOwner())
		return;

	MaxRelevancyDistance = DroneConfig ? DroneConfig->NetCullDistance : 15000.0f;

	// Apply cull distance
	GetOwner()->NetCullDistanceSquared = MaxRelevancyDistance * MaxRelevancyDistance;
}
