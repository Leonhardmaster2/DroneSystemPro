// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DroneTypes.h"
#include "DroneReplicationComponent.generated.h"

/**
 * Handles relevancy, prioritization, delta compression, and bandwidth caps
 * Optimizes network traffic for drone systems
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DRONESYSTEMPRO_API UDroneReplicationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDroneReplicationComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// Relevancy control
	UFUNCTION(BlueprintCallable, Category = "Replication")
	void UpdateRelevancy();

	UFUNCTION(BlueprintPure, Category = "Replication")
	bool IsRelevantTo(AActor* ViewingActor) const;

	// Priority control
	UFUNCTION(BlueprintPure, Category = "Replication")
	float GetReplicationPriority(AActor* ViewingActor) const;

	// Bandwidth optimization
	UFUNCTION(BlueprintCallable, Category = "Replication")
	void SetBandwidthLimit(float Limit);

	UFUNCTION(BlueprintPure, Category = "Replication")
	float GetCurrentBandwidthUsage() const { return CurrentBandwidthUsage; }

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Replication")
	void SetDroneConfig(UDroneConfig* NewConfig);

protected:
	// Configuration
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	UDroneConfig* DroneConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	float BasePriority;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	float DistancePriorityScale;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	float MaxRelevancyDistance;

	// State
	UPROPERTY()
	float BandwidthLimit;

	UPROPERTY()
	float CurrentBandwidthUsage;

	UPROPERTY()
	float LastRelevancyUpdate;

private:
	void OptimizeNetworkSettings();
	void ApplyRelevancySettings();
};
