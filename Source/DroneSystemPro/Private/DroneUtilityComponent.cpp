// Copyright Epic Games, Inc. All Rights Reserved.

#include "DroneUtilityComponent.h"
#include "DroneBatteryComponent.h"
#include "DroneMovementComponent.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

UDroneUtilityComponent::UDroneUtilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	bFlashlightEnabled = false;
}

void UDroneUtilityComponent::BeginPlay()
{
	Super::BeginPlay();

	// Try to find or create flashlight component
	if (GetOwner())
	{
		FlashlightComponent = GetOwner()->FindComponentByClass<USpotLightComponent>();

		if (!FlashlightComponent)
		{
			// Create flashlight if it doesn't exist
			FlashlightComponent = NewObject<USpotLightComponent>(GetOwner(), TEXT("Flashlight"));
			if (FlashlightComponent)
			{
				FlashlightComponent->RegisterComponent();
				FlashlightComponent->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
				FlashlightComponent->SetIntensity(5000.0f);
				FlashlightComponent->SetOuterConeAngle(35.0f);
				FlashlightComponent->SetAttenuationRadius(3000.0f);
				FlashlightComponent->SetVisibility(false);
			}
		}
	}

	UpdateFlashlightVisual();
}

void UDroneUtilityComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UDroneUtilityComponent, bFlashlightEnabled);
}

void UDroneUtilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UDroneUtilityComponent::ToggleFlashlight()
{
	SetFlashlightEnabled(!bFlashlightEnabled);
}

void UDroneUtilityComponent::SetFlashlightEnabled(bool bEnabled)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		bFlashlightEnabled = bEnabled;
		Multicast_SetFlashlight(bEnabled);
		NotifyBatteryComponent(bEnabled);
	}
	else
	{
		Server_SetFlashlight(bEnabled);
	}
}

void UDroneUtilityComponent::Server_SetFlashlight_Implementation(bool bEnabled)
{
	bFlashlightEnabled = bEnabled;
	Multicast_SetFlashlight(bEnabled);
	NotifyBatteryComponent(bEnabled);
}

bool UDroneUtilityComponent::Server_SetFlashlight_Validate(bool bEnabled)
{
	return true;
}

void UDroneUtilityComponent::Multicast_SetFlashlight_Implementation(bool bEnabled)
{
	bFlashlightEnabled = bEnabled;
	UpdateFlashlightVisual();
	OnFlashlightToggled.Broadcast(bEnabled);
}

void UDroneUtilityComponent::OnRep_FlashlightEnabled()
{
	UpdateFlashlightVisual();
	OnFlashlightToggled.Broadcast(bFlashlightEnabled);
}

float UDroneUtilityComponent::GetCompassHeading() const
{
	if (!GetOwner())
		return 0.0f;

	FRotator Rotation = GetOwner()->GetActorRotation();
	float Heading = Rotation.Yaw;

	// Normalize to 0-360
	while (Heading < 0.0f)
		Heading += 360.0f;
	while (Heading >= 360.0f)
		Heading -= 360.0f;

	return Heading;
}

FVector UDroneUtilityComponent::GetForwardDirection() const
{
	if (!GetOwner())
		return FVector::ForwardVector;

	return GetOwner()->GetActorForwardVector();
}

FRotator UDroneUtilityComponent::GetCurrentRotation() const
{
	if (!GetOwner())
		return FRotator::ZeroRotator;

	return GetOwner()->GetActorRotation();
}

FVector UDroneUtilityComponent::GetVelocity() const
{
	if (!GetOwner())
		return FVector::ZeroVector;

	UDroneMovementComponent* MovementComp = GetOwner()->FindComponentByClass<UDroneMovementComponent>();
	if (MovementComp)
	{
		return MovementComp->GetVelocity();
	}

	return FVector::ZeroVector;
}

float UDroneUtilityComponent::GetAltitude() const
{
	if (!GetOwner())
		return 0.0f;

	return GetOwner()->GetActorLocation().Z;
}

float UDroneUtilityComponent::GetSpeed() const
{
	return GetVelocity().Size();
}

void UDroneUtilityComponent::UpdateFlashlightVisual()
{
	if (FlashlightComponent)
	{
		FlashlightComponent->SetVisibility(bFlashlightEnabled);
	}
}

void UDroneUtilityComponent::NotifyBatteryComponent(bool bEnabled)
{
	if (!GetOwner())
		return;

	UDroneBatteryComponent* BatteryComp = GetOwner()->FindComponentByClass<UDroneBatteryComponent>();
	if (BatteryComp)
	{
		BatteryComp->SetFlashlightActive(bEnabled);
	}
}
