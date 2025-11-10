// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "JammingComponent.generated.h"

/**
 * Jamming component that affects drones in radius
 * Simulates signal loss, degrades vision fidelity, increases battery drain
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DRONESYSTEMPRO_API UJammingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UJammingComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Jamming control
	UFUNCTION(BlueprintCallable, Category = "Jamming")
	void SetJammingEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Jamming")
	bool IsJammingEnabled() const { return bJammingEnabled; }

	UFUNCTION(BlueprintCallable, Category = "Jamming")
	void SetJamStrength(float Strength);

	UFUNCTION(BlueprintPure, Category = "Jamming")
	float GetJamStrength() const { return JamStrength; }

	UFUNCTION(BlueprintCallable, Category = "Jamming")
	void SetJamRadius(float Radius);

	UFUNCTION(BlueprintPure, Category = "Jamming")
	float GetJamRadius() const { return JamRadius; }

	// Query
	UFUNCTION(BlueprintPure, Category = "Jamming")
	float GetJammingIntensityAtLocation(FVector Location) const;

	UFUNCTION(BlueprintPure, Category = "Jamming")
	TArray<AActor*> GetAffectedDrones() const;

protected:
	// Replication
	UPROPERTY(ReplicatedUsing=OnRep_JammingEnabled)
	bool bJammingEnabled;

	UFUNCTION()
	void OnRep_JammingEnabled();

	UPROPERTY(Replicated)
	float JamStrength;

	UPROPERTY(Replicated)
	float JamRadius;

	// Configuration
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	float DefaultJamStrength;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	float DefaultJamRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	float BatteryDrainMultiplier;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	float SensorRangeMultiplier;

private:
	void ApplyJammingEffects();
	void UpdateAffectedDrones();
};
