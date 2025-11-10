// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TerminalActor.generated.h"

class UHackingComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTerminalHacked, AActor*, Hacker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTerminalReset);

/**
 * Terminal actor that can be hacked by players
 * Provides interactive networked sequence with server validation
 */
UCLASS(Blueprintable)
class DRONESYSTEMPRO_API ATerminalActor : public AActor
{
	GENERATED_BODY()

public:
	ATerminalActor();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Terminal state
	UFUNCTION(BlueprintPure, Category = "Terminal")
	bool IsHacked() const { return bIsHacked; }

	UFUNCTION(BlueprintCallable, Category = "Terminal")
	void SetHacked(bool bHacked);

	UFUNCTION(BlueprintCallable, Category = "Terminal")
	void ResetTerminal();

	// Interaction
	UFUNCTION(BlueprintCallable, Category = "Terminal")
	bool CanBeHacked(AActor* Hacker) const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Terminal")
	FOnTerminalHacked OnTerminalHacked;

	UPROPERTY(BlueprintAssignable, Category = "Terminal")
	FOnTerminalReset OnTerminalReset;

protected:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* TerminalMesh;

	// State
	UPROPERTY(ReplicatedUsing=OnRep_IsHacked)
	bool bIsHacked;

	UFUNCTION()
	void OnRep_IsHacked();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	float InteractionRange;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	float HackDuration;

private:
	UFUNCTION()
	void OnHackCompleted(AActor* Hacker, AActor* Target);

	void UpdateVisuals();
};
