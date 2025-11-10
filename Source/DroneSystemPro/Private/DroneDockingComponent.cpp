// Copyright Epic Games, Inc. All Rights Reserved.

#include "DroneDockingComponent.h"
#include "DroneBase.h"
#include "DroneBatteryComponent.h"
#include "DroneAIController.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UDroneDockingComponent::UDroneDockingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	DockedDrone = nullptr;
	DockingRange = 200.0f;
	LowBatteryThreshold = 20.0f;
	bAutoRecallOnLowBattery = true;
	DockingOffset = FVector(0.0f, 0.0f, 100.0f);
}

void UDroneDockingComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UDroneDockingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UDroneDockingComponent, DockedDrone);
}

void UDroneDockingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	if (DockedDrone)
	{
		ProcessRecharging(DeltaTime);
	}

	if (bAutoRecallOnLowBattery)
	{
		CheckAutoRecall();
	}
}

bool UDroneDockingComponent::DockDrone(ADroneBase* Drone)
{
	if (!Drone || DockedDrone != nullptr)
		return false;

	if (!IsInDockingRange(Drone))
		return false;

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		DockedDrone = Drone;

		// Position drone at docking station
		FVector DockLocation = GetOwner()->GetActorLocation() + DockingOffset;
		Drone->SetActorLocation(DockLocation);

		// Disable drone
		Drone->SetActive(false);

		// Start recharging
		UDroneBatteryComponent* BatteryComp = Drone->GetDroneBattery();
		if (BatteryComp)
		{
			BatteryComp->StartRecharging();
		}

		Multicast_DroneDocked(Drone);
		return true;
	}
	else
	{
		Server_DockDrone(Drone);
		return true;
	}
}

void UDroneDockingComponent::UndockDrone()
{
	if (!DockedDrone)
		return;

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		ADroneBase* Drone = DockedDrone;

		// Stop recharging
		UDroneBatteryComponent* BatteryComp = Drone->GetDroneBattery();
		if (BatteryComp)
		{
			BatteryComp->StopRecharging();
		}

		// Enable drone
		Drone->SetActive(true);

		DockedDrone = nullptr;

		Multicast_DroneUndocked(Drone);
	}
	else
	{
		Server_UndockDrone();
	}
}

void UDroneDockingComponent::RecallDrone(ADroneBase* Drone)
{
	if (!Drone)
		return;

	// Command drone to return to docking station
	ADroneAIController* AIController = Cast<ADroneAIController>(Drone->GetController());
	if (AIController && GetOwner())
	{
		AIController->MoveToLocation(GetOwner()->GetActorLocation(), DockingRange);
	}
}

void UDroneDockingComponent::Server_DockDrone_Implementation(ADroneBase* Drone)
{
	DockDrone(Drone);
}

bool UDroneDockingComponent::Server_DockDrone_Validate(ADroneBase* Drone)
{
	return Drone != nullptr && IsInDockingRange(Drone);
}

void UDroneDockingComponent::Server_UndockDrone_Implementation()
{
	UndockDrone();
}

bool UDroneDockingComponent::Server_UndockDrone_Validate()
{
	return DockedDrone != nullptr;
}

void UDroneDockingComponent::Multicast_DroneDocked_Implementation(ADroneBase* Drone)
{
	OnDroneDocked.Broadcast(Drone);
}

void UDroneDockingComponent::Multicast_DroneUndocked_Implementation(ADroneBase* Drone)
{
	OnDroneUndocked.Broadcast(Drone);
}

void UDroneDockingComponent::ProcessRecharging(float DeltaTime)
{
	if (!DockedDrone)
		return;

	UDroneBatteryComponent* BatteryComp = DockedDrone->GetDroneBattery();
	if (!BatteryComp)
		return;

	// Check if fully recharged
	if (BatteryComp->GetBatteryPercent() >= 100.0f)
	{
		// Optionally auto-undock when fully charged
		// UndockDrone();
	}
}

bool UDroneDockingComponent::IsInDockingRange(ADroneBase* Drone) const
{
	if (!Drone || !GetOwner())
		return false;

	float DistSq = FVector::DistSquared(GetOwner()->GetActorLocation(), Drone->GetActorLocation());
	float MaxRangeSq = DockingRange * DockingRange;

	return DistSq <= MaxRangeSq;
}

void UDroneDockingComponent::CheckAutoRecall()
{
	// This would check all drones registered with this station
	// and recall them if their battery is low
	// Simplified implementation - would need drone registration system
}
