// Copyright Epic Games, Inc. All Rights Reserved.

#include "TerminalActor.h"
#include "HackingComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

ATerminalActor::ATerminalActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	TerminalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TerminalMesh"));
	RootComponent = TerminalMesh;

	bIsHacked = false;
	InteractionRange = 300.0f;
	HackDuration = 5.0f;
}

void ATerminalActor::BeginPlay()
{
	Super::BeginPlay();

	UpdateVisuals();
}

void ATerminalActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATerminalActor, bIsHacked);
}

void ATerminalActor::SetHacked(bool bHacked)
{
	if (HasAuthority())
	{
		bIsHacked = bHacked;
		UpdateVisuals();

		if (bIsHacked)
		{
			OnTerminalHacked.Broadcast(nullptr);
		}
	}
}

void ATerminalActor::ResetTerminal()
{
	if (HasAuthority())
	{
		bIsHacked = false;
		UpdateVisuals();
		OnTerminalReset.Broadcast();
	}
}

bool ATerminalActor::CanBeHacked(AActor* Hacker) const
{
	if (bIsHacked || !Hacker)
		return false;

	float DistSq = FVector::DistSquared(GetActorLocation(), Hacker->GetActorLocation());
	float MaxRangeSq = InteractionRange * InteractionRange;

	return DistSq <= MaxRangeSq;
}

void ATerminalActor::OnRep_IsHacked()
{
	UpdateVisuals();

	if (bIsHacked)
	{
		OnTerminalHacked.Broadcast(nullptr);
	}
	else
	{
		OnTerminalReset.Broadcast();
	}
}

void ATerminalActor::OnHackCompleted(AActor* Hacker, AActor* Target)
{
	if (Target == this && HasAuthority())
	{
		SetHacked(true);
	}
}

void ATerminalActor::UpdateVisuals()
{
	// Blueprint can implement visual changes based on bIsHacked
	// E.g., change material, spawn particles, etc.
}
