// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DroneTypes.h"
#include "DroneUtilityComponent.generated.h"

class USpotLightComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFlashlightToggled, bool, bIsOn);

/**
 * Provides utility features like flashlight and compass
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DRONESYSTEMPRO_API UDroneUtilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDroneUtilityComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Flashlight
	UFUNCTION(BlueprintCallable, Category = "Utility")
	void ToggleFlashlight();

	UFUNCTION(BlueprintCallable, Category = "Utility")
	void SetFlashlightEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Utility")
	bool IsFlashlightEnabled() const { return bFlashlightEnabled; }

	// Compass / Direction
	UFUNCTION(BlueprintPure, Category = "Utility")
	float GetCompassHeading() const;

	UFUNCTION(BlueprintPure, Category = "Utility")
	FVector GetForwardDirection() const;

	UFUNCTION(BlueprintPure, Category = "Utility")
	FRotator GetCurrentRotation() const;

	// HUD data
	UFUNCTION(BlueprintPure, Category = "Utility")
	FVector GetVelocity() const;

	UFUNCTION(BlueprintPure, Category = "Utility")
	float GetAltitude() const;

	UFUNCTION(BlueprintPure, Category = "Utility")
	float GetSpeed() const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Utility")
	FOnFlashlightToggled OnFlashlightToggled;

protected:
	// Network RPCs
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetFlashlight(bool bEnabled);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetFlashlight(bool bEnabled);

	// Replication
	UPROPERTY(ReplicatedUsing=OnRep_FlashlightEnabled)
	bool bFlashlightEnabled;

	UFUNCTION()
	void OnRep_FlashlightEnabled();

	// Flashlight component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpotLightComponent* FlashlightComponent;

private:
	void UpdateFlashlightVisual();
	void NotifyBatteryComponent(bool bEnabled);
};
