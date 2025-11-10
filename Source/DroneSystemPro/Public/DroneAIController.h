// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "DroneTypes.h"
#include "DroneAIController.generated.h"

class UDroneBehaviorProfile;

/**
 * AI Controller for autonomous drone behavior
 * Supports data-driven behaviors: Patrol, Follow, Scan, Attack-Mark
 */
UCLASS()
class DRONESYSTEMPRO_API ADroneAIController : public AAIController
{
	GENERATED_BODY()

public:
	ADroneAIController();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnPossess(APawn* InPawn) override;

public:
	// Behavior control
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetBehaviorProfile(UDroneBehaviorProfile* NewProfile);

	UFUNCTION(BlueprintPure, Category = "AI")
	UDroneBehaviorProfile* GetBehaviorProfile() const { return BehaviorProfile; }

	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetBehaviorType(EDroneBehaviorType NewType);

	UFUNCTION(BlueprintPure, Category = "AI")
	EDroneBehaviorType GetCurrentBehavior() const { return CurrentBehavior; }

	// Target management
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetFollowTarget(AActor* Target);

	UFUNCTION(BlueprintPure, Category = "AI")
	AActor* GetFollowTarget() const { return FollowTarget; }

	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetPatrolCenter(FVector Center);

	UFUNCTION(BlueprintCallable, Category = "AI")
	void AddPatrolPoint(FVector Point);

	UFUNCTION(BlueprintCallable, Category = "AI")
	void ClearPatrolPoints();

protected:
	// Behavior implementation
	void ExecuteIdleBehavior(float DeltaTime);
	void ExecutePatrolBehavior(float DeltaTime);
	void ExecuteFollowBehavior(float DeltaTime);
	void ExecuteScanBehavior(float DeltaTime);
	void ExecuteAttackMarkBehavior(float DeltaTime);

	// Helper functions
	void UpdatePerception(float DeltaTime);
	void FindAndMarkEnemies();
	FVector GetNextPatrolPoint();
	bool HasReachedTarget(float Tolerance = 100.0f) const;

	// Configuration
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	UDroneBehaviorProfile* BehaviorProfile;

	// State
	UPROPERTY()
	EDroneBehaviorType CurrentBehavior;

	UPROPERTY()
	AActor* FollowTarget;

	UPROPERTY()
	FVector PatrolCenter;

	UPROPERTY()
	TArray<FVector> PatrolPoints;

	UPROPERTY()
	int32 CurrentPatrolIndex;

	UPROPERTY()
	FVector ScanCenter;

	UPROPERTY()
	float ScanStartTime;

	UPROPERTY()
	TArray<AActor*> DetectedEnemies;

	UPROPERTY()
	float LastPerceptionUpdate;

	UPROPERTY()
	float PerceptionUpdateInterval;
};
