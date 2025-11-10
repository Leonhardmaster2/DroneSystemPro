// Copyright Epic Games, Inc. All Rights Reserved.

#include "HackingComponent.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

UHackingComponent::UHackingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	HackRange = 500.0f;
	DefaultHackDuration = 5.0f;
}

void UHackingComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UHackingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHackingComponent, CurrentSession);
}

void UHackingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	if (CurrentSession.bIsActive)
	{
		ProcessHacking(DeltaTime);
	}
}

bool UHackingComponent::StartHack(AActor* Target, float Duration)
{
	if (!Target || IsHacking())
		return false;

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (!ValidateHackTarget(Target))
			return false;

		// Initialize session
		CurrentSession.HackerActor = GetOwner();
		CurrentSession.TargetActor = Target;
		CurrentSession.Duration = Duration;
		CurrentSession.Progress = 0.0f;
		CurrentSession.StartTime = GetWorld()->GetTimeSeconds();
		CurrentSession.bIsActive = true;

		Multicast_HackStarted(GetOwner(), Target);
		return true;
	}
	else
	{
		Server_StartHack(Target, Duration);
		return true;
	}
}

void UHackingComponent::CancelHack()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (CurrentSession.bIsActive)
		{
			FailHack();
		}
	}
	else
	{
		Server_CancelHack();
	}
}

bool UHackingComponent::IsHacking() const
{
	return CurrentSession.bIsActive;
}

float UHackingComponent::GetHackProgress() const
{
	return CurrentSession.Progress;
}

AActor* UHackingComponent::GetHackTarget() const
{
	return CurrentSession.TargetActor;
}

void UHackingComponent::Server_StartHack_Implementation(AActor* Target, float Duration)
{
	StartHack(Target, Duration);
}

bool UHackingComponent::Server_StartHack_Validate(AActor* Target, float Duration)
{
	if (!Target || !GetOwner())
		return false;

	// Validate range
	float DistSq = FVector::DistSquared(GetOwner()->GetActorLocation(), Target->GetActorLocation());
	float MaxRangeSq = HackRange * HackRange;

	return DistSq <= (MaxRangeSq * 1.2f); // Allow some tolerance
}

void UHackingComponent::Server_CancelHack_Implementation()
{
	CancelHack();
}

bool UHackingComponent::Server_CancelHack_Validate()
{
	return true;
}

void UHackingComponent::Multicast_HackStarted_Implementation(AActor* Hacker, AActor* Target)
{
	OnHackStarted.Broadcast(Hacker, Target);
}

void UHackingComponent::Client_HackProgress_Implementation(float Progress)
{
	OnHackProgress.Broadcast(CurrentSession.HackerActor, CurrentSession.TargetActor, Progress);
}

void UHackingComponent::Multicast_HackCompleted_Implementation(AActor* Hacker, AActor* Target)
{
	OnHackCompleted.Broadcast(Hacker, Target);
}

void UHackingComponent::Multicast_HackFailed_Implementation(AActor* Hacker, AActor* Target)
{
	OnHackFailed.Broadcast(Hacker, Target);
}

void UHackingComponent::ProcessHacking(float DeltaTime)
{
	if (!GetWorld() || !CurrentSession.bIsActive)
		return;

	// Check if hacker is still in range
	if (!ValidateHackTarget(CurrentSession.TargetActor))
	{
		FailHack();
		return;
	}

	// Update progress
	float CurrentTime = GetWorld()->GetTimeSeconds();
	float ElapsedTime = CurrentTime - CurrentSession.StartTime;
	CurrentSession.Progress = FMath::Clamp(ElapsedTime / CurrentSession.Duration, 0.0f, 1.0f);

	// Send progress update to client
	if (GetOwner()->GetLocalRole() == ROLE_Authority)
	{
		OnHackProgress.Broadcast(CurrentSession.HackerActor, CurrentSession.TargetActor, CurrentSession.Progress);
	}

	// Check if hack is complete
	if (CurrentSession.Progress >= 1.0f)
	{
		CompleteHack();
	}
}

bool UHackingComponent::ValidateHackTarget(AActor* Target) const
{
	if (!Target || !GetOwner())
		return false;

	// Check range
	float DistSq = FVector::DistSquared(GetOwner()->GetActorLocation(), Target->GetActorLocation());
	float MaxRangeSq = HackRange * HackRange;

	return DistSq <= MaxRangeSq;
}

void UHackingComponent::CompleteHack()
{
	AActor* Hacker = CurrentSession.HackerActor;
	AActor* Target = CurrentSession.TargetActor;

	CurrentSession = FHackingSession(); // Reset session

	Multicast_HackCompleted(Hacker, Target);
}

void UHackingComponent::FailHack()
{
	AActor* Hacker = CurrentSession.HackerActor;
	AActor* Target = CurrentSession.TargetActor;

	CurrentSession = FHackingSession(); // Reset session

	Multicast_HackFailed(Hacker, Target);
}
