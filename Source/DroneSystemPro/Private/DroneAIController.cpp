// Copyright Epic Games, Inc. All Rights Reserved.

#include "DroneAIController.h"
#include "DroneBase.h"
#include "DroneMovementComponent.h"
#include "DroneMarkingComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "NavigationSystem.h"

ADroneAIController::ADroneAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	CurrentBehavior = EDroneBehaviorType::Idle;
	FollowTarget = nullptr;
	PatrolCenter = FVector::ZeroVector;
	CurrentPatrolIndex = 0;
	ScanCenter = FVector::ZeroVector;
	ScanStartTime = 0.0f;
	LastPerceptionUpdate = 0.0f;
	PerceptionUpdateInterval = 0.5f;
}

void ADroneAIController::BeginPlay()
{
	Super::BeginPlay();

	if (BehaviorProfile)
	{
		CurrentBehavior = BehaviorProfile->BehaviorType;
	}
}

void ADroneAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!GetPawn())
		return;

	// Update perception
	UpdatePerception(DeltaTime);

	// Execute current behavior
	switch (CurrentBehavior)
	{
	case EDroneBehaviorType::Idle:
		ExecuteIdleBehavior(DeltaTime);
		break;
	case EDroneBehaviorType::Patrol:
		ExecutePatrolBehavior(DeltaTime);
		break;
	case EDroneBehaviorType::Follow:
		ExecuteFollowBehavior(DeltaTime);
		break;
	case EDroneBehaviorType::Scan:
		ExecuteScanBehavior(DeltaTime);
		break;
	case EDroneBehaviorType::AttackMark:
		ExecuteAttackMarkBehavior(DeltaTime);
		break;
	}
}

void ADroneAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Initialize patrol center to current location
	if (InPawn)
	{
		PatrolCenter = InPawn->GetActorLocation();
	}
}

void ADroneAIController::SetBehaviorProfile(UDroneBehaviorProfile* NewProfile)
{
	BehaviorProfile = NewProfile;

	if (BehaviorProfile)
	{
		CurrentBehavior = BehaviorProfile->BehaviorType;
	}
}

void ADroneAIController::SetBehaviorType(EDroneBehaviorType NewType)
{
	CurrentBehavior = NewType;

	// Reset state variables when switching behavior
	CurrentPatrolIndex = 0;
	ScanStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
}

void ADroneAIController::SetFollowTarget(AActor* Target)
{
	FollowTarget = Target;
}

void ADroneAIController::SetPatrolCenter(FVector Center)
{
	PatrolCenter = Center;
}

void ADroneAIController::AddPatrolPoint(FVector Point)
{
	PatrolPoints.Add(Point);
}

void ADroneAIController::ClearPatrolPoints()
{
	PatrolPoints.Empty();
	CurrentPatrolIndex = 0;
}

void ADroneAIController::ExecuteIdleBehavior(float DeltaTime)
{
	// Stay in place, minimal movement
	// Could add hover animation here
}

void ADroneAIController::ExecutePatrolBehavior(float DeltaTime)
{
	if (!GetPawn())
		return;

	FVector TargetLocation;

	if (PatrolPoints.Num() > 0)
	{
		// Use defined patrol points
		TargetLocation = GetNextPatrolPoint();
	}
	else
	{
		// Generate patrol point in radius around patrol center
		float Radius = BehaviorProfile ? BehaviorProfile->PatrolRadius : 1000.0f;
		FVector RandomOffset = FMath::VRand() * FMath::FRandRange(0.0f, Radius);
		RandomOffset.Z = FMath::FRandRange(-200.0f, 200.0f);
		TargetLocation = PatrolCenter + RandomOffset;
	}

	// Move to target
	if (HasReachedTarget())
	{
		if (PatrolPoints.Num() > 0)
		{
			CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
		}

		// Generate new patrol point
		float Radius = BehaviorProfile ? BehaviorProfile->PatrolRadius : 1000.0f;
		FVector RandomOffset = FMath::VRand() * FMath::FRandRange(0.0f, Radius);
		RandomOffset.Z = FMath::FRandRange(-200.0f, 200.0f);
		TargetLocation = PatrolCenter + RandomOffset;
	}

	MoveToLocation(TargetLocation, 50.0f);
}

void ADroneAIController::ExecuteFollowBehavior(float DeltaTime)
{
	if (!FollowTarget || !GetPawn())
		return;

	float FollowDistance = BehaviorProfile ? BehaviorProfile->FollowDistance : 500.0f;
	FVector TargetLocation = FollowTarget->GetActorLocation();
	FVector DirectionToTarget = (GetPawn()->GetActorLocation() - TargetLocation).GetSafeNormal();
	FVector FollowLocation = TargetLocation + (DirectionToTarget * FollowDistance);

	MoveToLocation(FollowLocation, FollowDistance * 0.2f);
}

void ADroneAIController::ExecuteScanBehavior(float DeltaTime)
{
	if (!GetPawn() || !GetWorld())
		return;

	float CurrentTime = GetWorld()->GetTimeSeconds();
	float ScanDuration = BehaviorProfile ? BehaviorProfile->ScanDuration : 10.0f;

	// Initialize scan if just started
	if (ScanStartTime == 0.0f)
	{
		ScanStartTime = CurrentTime;
		ScanCenter = GetPawn()->GetActorLocation();
	}

	// Check if scan is complete
	if (CurrentTime - ScanStartTime >= ScanDuration)
	{
		// Scan complete, return to idle or switch behavior
		SetBehaviorType(EDroneBehaviorType::Idle);
		return;
	}

	// Rotate in place and scan area
	float ScanRadius = BehaviorProfile ? BehaviorProfile->ScanRadius : 1500.0f;
	float Progress = (CurrentTime - ScanStartTime) / ScanDuration;
	float Angle = Progress * 360.0f;

	FVector Offset = FVector(
		FMath::Cos(FMath::DegreesToRadians(Angle)) * ScanRadius * 0.5f,
		FMath::Sin(FMath::DegreesToRadians(Angle)) * ScanRadius * 0.5f,
		0.0f
	);

	FVector TargetLocation = ScanCenter + Offset;
	MoveToLocation(TargetLocation, 100.0f);

	// Mark detected enemies during scan
	if (BehaviorProfile && BehaviorProfile->bAutoMarkEnemies)
	{
		FindAndMarkEnemies();
	}
}

void ADroneAIController::ExecuteAttackMarkBehavior(float DeltaTime)
{
	if (!GetPawn())
		return;

	// Find and mark enemies
	FindAndMarkEnemies();

	// Move towards detected enemies
	if (DetectedEnemies.Num() > 0)
	{
		AActor* ClosestEnemy = nullptr;
		float ClosestDistSq = FLT_MAX;

		for (AActor* Enemy : DetectedEnemies)
		{
			if (!Enemy)
				continue;

			float DistSq = FVector::DistSquared(GetPawn()->GetActorLocation(), Enemy->GetActorLocation());
			if (DistSq < ClosestDistSq)
			{
				ClosestDistSq = DistSq;
				ClosestEnemy = Enemy;
			}
		}

		if (ClosestEnemy)
		{
			float AttackDistance = 1000.0f; // Maintain distance while marking
			FVector TargetLocation = ClosestEnemy->GetActorLocation();
			FVector DirectionToTarget = (GetPawn()->GetActorLocation() - TargetLocation).GetSafeNormal();
			FVector AttackPosition = TargetLocation + (DirectionToTarget * AttackDistance);

			MoveToLocation(AttackPosition, 100.0f);
		}
	}
	else
	{
		// No enemies detected, patrol to find them
		ExecutePatrolBehavior(DeltaTime);
	}
}

void ADroneAIController::UpdatePerception(float DeltaTime)
{
	if (!GetWorld())
		return;

	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastPerceptionUpdate < PerceptionUpdateInterval)
		return;

	LastPerceptionUpdate = CurrentTime;

	// Clear old detections
	DetectedEnemies.Empty();

	// This would integrate with UE's perception system in a full implementation
	// For now, simple radius check for characters
	float DetectionRadius = 3000.0f;
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (!Actor || Actor == GetPawn())
			continue;

		float DistSq = FVector::DistSquared(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
		if (DistSq <= (DetectionRadius * DetectionRadius))
		{
			DetectedEnemies.Add(Actor);
		}
	}
}

void ADroneAIController::FindAndMarkEnemies()
{
	ADroneBase* Drone = Cast<ADroneBase>(GetPawn());
	if (!Drone)
		return;

	UDroneMarkingComponent* MarkingComp = Drone->GetDroneMarking();
	if (!MarkingComp)
		return;

	for (AActor* Enemy : DetectedEnemies)
	{
		if (Enemy)
		{
			MarkingComp->MarkTarget(Enemy);
		}
	}
}

FVector ADroneAIController::GetNextPatrolPoint()
{
	if (PatrolPoints.Num() == 0)
		return PatrolCenter;

	CurrentPatrolIndex = FMath::Clamp(CurrentPatrolIndex, 0, PatrolPoints.Num() - 1);
	return PatrolPoints[CurrentPatrolIndex];
}

bool ADroneAIController::HasReachedTarget(float Tolerance) const
{
	if (!GetPawn())
		return false;

	FVector CurrentLocation = GetPawn()->GetActorLocation();
	FVector TargetLocation = GetPathFollowingComponent() ? GetPathFollowingComponent()->GetCurrentTargetLocation() : CurrentLocation;

	float DistSq = FVector::DistSquared(CurrentLocation, TargetLocation);
	return DistSq <= (Tolerance * Tolerance);
}
