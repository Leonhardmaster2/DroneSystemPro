// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DroneTypes.h"
#include "HackingComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHackStarted, AActor*, Hacker, AActor*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHackProgress, AActor*, Hacker, AActor*, Target, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHackCompleted, AActor*, Hacker, AActor*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHackFailed, AActor*, Hacker, AActor*, Target);

/**
 * Hacking component for multiplayer hack minigame flow
 * Server validates progress and sends success/failure events
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DRONESYSTEMPRO_API UHackingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHackingComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Hacking control
	UFUNCTION(BlueprintCallable, Category = "Hacking")
	bool StartHack(AActor* Target, float Duration = 5.0f);

	UFUNCTION(BlueprintCallable, Category = "Hacking")
	void CancelHack();

	UFUNCTION(BlueprintPure, Category = "Hacking")
	bool IsHacking() const;

	UFUNCTION(BlueprintPure, Category = "Hacking")
	float GetHackProgress() const;

	UFUNCTION(BlueprintPure, Category = "Hacking")
	AActor* GetHackTarget() const;

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Hacking")
	void SetHackRange(float Range) { HackRange = Range; }

	UFUNCTION(BlueprintPure, Category = "Hacking")
	float GetHackRange() const { return HackRange; }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Hacking")
	FOnHackStarted OnHackStarted;

	UPROPERTY(BlueprintAssignable, Category = "Hacking")
	FOnHackProgress OnHackProgress;

	UPROPERTY(BlueprintAssignable, Category = "Hacking")
	FOnHackCompleted OnHackCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Hacking")
	FOnHackFailed OnHackFailed;

protected:
	// Network RPCs
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StartHack(AActor* Target, float Duration);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_CancelHack();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HackStarted(AActor* Hacker, AActor* Target);

	UFUNCTION(Client, Reliable)
	void Client_HackProgress(float Progress);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HackCompleted(AActor* Hacker, AActor* Target);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HackFailed(AActor* Hacker, AActor* Target);

	// Replication
	UPROPERTY(Replicated)
	FHackingSession CurrentSession;

	// Configuration
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	float HackRange;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	float DefaultHackDuration;

private:
	void ProcessHacking(float DeltaTime);
	bool ValidateHackTarget(AActor* Target) const;
	void CompleteHack();
	void FailHack();
};
