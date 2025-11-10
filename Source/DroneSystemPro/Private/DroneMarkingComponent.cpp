// Copyright Epic Games, Inc. All Rights Reserved.

#include "DroneMarkingComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UDroneMarkingComponent::UDroneMarkingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	MarkTag = FName("DroneMarked");
}

void UDroneMarkingComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UDroneMarkingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UDroneMarkingComponent, MarkedTargets);
}

void UDroneMarkingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	UpdateMarkedTargets(DeltaTime);
}

void UDroneMarkingComponent::MarkTarget(AActor* Target)
{
	if (!Target || !GetOwner())
		return;

	// Check range
	if (!IsTargetInRange(Target))
		return;

	if (GetOwner()->HasAuthority())
	{
		// Check if already marked
		bool bAlreadyMarked = false;
		for (FMarkedTarget& Marked : MarkedTargets)
		{
			if (Marked.Target == Target)
			{
				// Refresh mark time
				Marked.MarkTime = GetWorld()->GetTimeSeconds();
				bAlreadyMarked = true;
				break;
			}
		}

		if (!bAlreadyMarked)
		{
			float Duration = DroneConfig ? DroneConfig->MarkDuration : 10.0f;
			FMarkedTarget NewMark(Target, Duration);
			NewMark.MarkTime = GetWorld()->GetTimeSeconds();
			MarkedTargets.Add(NewMark);

			// Apply visuals
			ApplyMarkVisuals(Target, true);

			// Broadcast event
			Multicast_MarkTarget(Target);
		}
	}
	else
	{
		Server_MarkTarget(Target);
	}
}

void UDroneMarkingComponent::UnmarkTarget(AActor* Target)
{
	if (!Target || !GetOwner())
		return;

	if (GetOwner()->HasAuthority())
	{
		int32 Index = MarkedTargets.IndexOfByPredicate([Target](const FMarkedTarget& Mark) {
			return Mark.Target == Target;
		});

		if (Index != INDEX_NONE)
		{
			MarkedTargets.RemoveAt(Index);
			ApplyMarkVisuals(Target, false);
			Multicast_UnmarkTarget(Target);
		}
	}
	else
	{
		Server_UnmarkTarget(Target);
	}
}

void UDroneMarkingComponent::MarkTargetInCrosshair()
{
	AActor* Target = GetTargetInCrosshair();
	if (Target)
	{
		MarkTarget(Target);
	}
}

TArray<AActor*> UDroneMarkingComponent::GetMarkedTargets() const
{
	TArray<AActor*> Result;
	for (const FMarkedTarget& Mark : MarkedTargets)
	{
		if (Mark.IsValid())
		{
			Result.Add(Mark.Target);
		}
	}
	return Result;
}

bool UDroneMarkingComponent::IsTargetMarked(AActor* Target) const
{
	if (!Target)
		return false;

	for (const FMarkedTarget& Mark : MarkedTargets)
	{
		if (Mark.Target == Target)
			return true;
	}

	return false;
}

void UDroneMarkingComponent::SetDroneConfig(UDroneConfig* NewConfig)
{
	DroneConfig = NewConfig;
}

float UDroneMarkingComponent::GetMarkingRange() const
{
	return DroneConfig ? DroneConfig->MarkingRange : 2500.0f;
}

void UDroneMarkingComponent::Server_MarkTarget_Implementation(AActor* Target)
{
	MarkTarget(Target);
}

bool UDroneMarkingComponent::Server_MarkTarget_Validate(AActor* Target)
{
	// Validate target is in range
	if (!Target || !GetOwner())
		return false;

	float DistSq = FVector::DistSquared(GetOwner()->GetActorLocation(), Target->GetActorLocation());
	float MaxRangeSq = GetMarkingRange() * GetMarkingRange();

	return DistSq <= (MaxRangeSq * 1.2f); // Allow some tolerance
}

void UDroneMarkingComponent::Server_UnmarkTarget_Implementation(AActor* Target)
{
	UnmarkTarget(Target);
}

bool UDroneMarkingComponent::Server_UnmarkTarget_Validate(AActor* Target)
{
	return Target != nullptr;
}

void UDroneMarkingComponent::Multicast_MarkTarget_Implementation(AActor* Target)
{
	if (!Target)
		return;

	ApplyMarkVisuals(Target, true);
	OnTargetMarked.Broadcast(Target);
}

void UDroneMarkingComponent::Multicast_UnmarkTarget_Implementation(AActor* Target)
{
	if (!Target)
		return;

	ApplyMarkVisuals(Target, false);
	OnTargetUnmarked.Broadcast(Target);
}

void UDroneMarkingComponent::UpdateMarkedTargets(float DeltaTime)
{
	if (!GetWorld())
		return;

	float CurrentTime = GetWorld()->GetTimeSeconds();
	TArray<AActor*> ToUnmark;

	// Check for expired marks
	for (FMarkedTarget& Mark : MarkedTargets)
	{
		if (!Mark.IsValid() || Mark.IsExpired(CurrentTime))
		{
			ToUnmark.Add(Mark.Target);
		}
	}

	// Remove expired marks
	for (AActor* Target : ToUnmark)
	{
		UnmarkTarget(Target);
	}
}

AActor* UDroneMarkingComponent::GetTargetInCrosshair() const
{
	if (!GetOwner())
		return nullptr;

	// Get camera component
	UCameraComponent* Camera = GetOwner()->FindComponentByClass<UCameraComponent>();
	if (!Camera)
		return nullptr;

	// Perform line trace from camera
	FVector Start = Camera->GetComponentLocation();
	FVector Forward = Camera->GetForwardVector();
	FVector End = Start + (Forward * GetMarkingRange());

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && (HitActor->IsA(APawn::StaticClass()) || HitActor->IsA(ACharacter::StaticClass())))
		{
			return HitActor;
		}
	}

	return nullptr;
}

bool UDroneMarkingComponent::IsTargetInRange(AActor* Target) const
{
	if (!Target || !GetOwner())
		return false;

	float DistSq = FVector::DistSquared(GetOwner()->GetActorLocation(), Target->GetActorLocation());
	float MaxRangeSq = GetMarkingRange() * GetMarkingRange();

	return DistSq <= MaxRangeSq;
}

void UDroneMarkingComponent::ApplyMarkVisuals(AActor* Target, bool bMarked)
{
	if (!Target)
		return;

	// Add/remove tag
	if (bMarked)
	{
		if (!Target->Tags.Contains(MarkTag))
		{
			Target->Tags.Add(MarkTag);
		}
	}
	else
	{
		Target->Tags.Remove(MarkTag);
	}

	// In a full implementation, this would enable custom depth/stencil rendering
	// for outline effect through walls
	// Blueprint can handle visual effects based on the tag
}
