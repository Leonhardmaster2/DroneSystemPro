// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DroneTypes.h"
#include "DroneMarkingComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetMarked, AActor*, MarkedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetUnmarked, AActor*, UnmarkedActor);

/**
 * Handles marking/tagging of enemies with outline through walls
 * Uses AActor tags + networked marking events with timeouts
 * Optimized by sending only IDs + timestamps
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DRONESYSTEMPRO_API UDroneMarkingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDroneMarkingComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Marking control
	UFUNCTION(BlueprintCallable, Category = "Marking")
	void MarkTarget(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "Marking")
	void UnmarkTarget(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "Marking")
	void MarkTargetInCrosshair();

	UFUNCTION(BlueprintPure, Category = "Marking")
	TArray<AActor*> GetMarkedTargets() const;

	UFUNCTION(BlueprintPure, Category = "Marking")
	bool IsTargetMarked(AActor* Target) const;

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Marking")
	void SetDroneConfig(UDroneConfig* NewConfig);

	UFUNCTION(BlueprintPure, Category = "Marking")
	float GetMarkingRange() const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Marking")
	FOnTargetMarked OnTargetMarked;

	UPROPERTY(BlueprintAssignable, Category = "Marking")
	FOnTargetUnmarked OnTargetUnmarked;

protected:
	// Network RPCs
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MarkTarget(AActor* Target);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_UnmarkTarget(AActor* Target);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_MarkTarget(AActor* Target);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UnmarkTarget(AActor* Target);

	// Replication
	UPROPERTY(Replicated)
	TArray<FMarkedTarget> MarkedTargets;

	// Configuration
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	UDroneConfig* DroneConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	FName MarkTag;

private:
	void UpdateMarkedTargets(float DeltaTime);
	AActor* GetTargetInCrosshair() const;
	bool IsTargetInRange(AActor* Target) const;
	void ApplyMarkVisuals(AActor* Target, bool bMarked);
};
