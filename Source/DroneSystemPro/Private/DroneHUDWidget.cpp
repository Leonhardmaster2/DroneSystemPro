// Copyright Epic Games, Inc. All Rights Reserved.

#include "DroneHUDWidget.h"
#include "DroneBase.h"
#include "DroneBatteryComponent.h"
#include "DroneVisionComponent.h"
#include "DroneMarkingComponent.h"
#include "DroneUtilityComponent.h"
#include "DroneMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

UDroneHUDWidget::UDroneHUDWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	OwningDrone = nullptr;
	BatteryComponent = nullptr;
	VisionComponent = nullptr;
	MarkingComponent = nullptr;
	UtilityComponent = nullptr;

	LowBatteryThreshold = 30.0f;
	CriticalBatteryThreshold = 15.0f;

	NormalBatteryColor = FLinearColor::Green;
	LowBatteryColor = FLinearColor::Yellow;
	CriticalBatteryColor = FLinearColor::Red;
}

void UDroneHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Try to find drone if not set
	if (!OwningDrone)
	{
		APlayerController* PC = GetOwningPlayer();
		if (PC)
		{
			APawn* Pawn = PC->GetPawn();
			OwningDrone = Cast<ADroneBase>(Pawn);
			if (OwningDrone)
			{
				CacheComponents();
			}
		}
	}
}

void UDroneHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Auto-find drone if lost
	if (!OwningDrone || !OwningDrone->IsValidLowLevel())
	{
		APlayerController* PC = GetOwningPlayer();
		if (PC)
		{
			APawn* Pawn = PC->GetPawn();
			if (Pawn != OwningDrone)
			{
				OwningDrone = Cast<ADroneBase>(Pawn);
				if (OwningDrone)
				{
					CacheComponents();
				}
			}
		}
	}
}

void UDroneHUDWidget::SetDrone(ADroneBase* InDrone)
{
	OwningDrone = InDrone;
	CacheComponents();
}

float UDroneHUDWidget::GetBatteryPercent() const
{
	if (BatteryComponent)
	{
		return BatteryComponent->GetBatteryPercent();
	}
	return 0.0f;
}

FLinearColor UDroneHUDWidget::GetBatteryColor() const
{
	float Percent = GetBatteryPercent();

	if (Percent <= CriticalBatteryThreshold)
	{
		return CriticalBatteryColor;
	}
	else if (Percent <= LowBatteryThreshold)
	{
		return LowBatteryColor;
	}

	return NormalBatteryColor;
}

bool UDroneHUDWidget::IsBatteryLow() const
{
	return GetBatteryPercent() <= LowBatteryThreshold;
}

bool UDroneHUDWidget::IsBatteryCritical() const
{
	return GetBatteryPercent() <= CriticalBatteryThreshold;
}

float UDroneHUDWidget::GetSpeed() const
{
	if (UtilityComponent)
	{
		return UtilityComponent->GetSpeed();
	}
	return 0.0f;
}

float UDroneHUDWidget::GetAltitude() const
{
	if (UtilityComponent)
	{
		return UtilityComponent->GetAltitude();
	}
	return 0.0f;
}

float UDroneHUDWidget::GetCompassHeading() const
{
	if (UtilityComponent)
	{
		return UtilityComponent->GetCompassHeading();
	}
	return 0.0f;
}

FString UDroneHUDWidget::GetSpeedText() const
{
	float Speed = GetSpeed();
	return FString::Printf(TEXT("%.0f"), Speed);
}

FString UDroneHUDWidget::GetAltitudeText() const
{
	float Altitude = GetAltitude();
	return FString::Printf(TEXT("%.0f"), Altitude);
}

EDroneSpeedMode UDroneHUDWidget::GetSpeedMode() const
{
	if (OwningDrone)
	{
		UDroneMovementComponent* MovementComp = OwningDrone->GetDroneMovement();
		if (MovementComp)
		{
			return MovementComp->GetSpeedMode();
		}
	}
	return EDroneSpeedMode::Low;
}

EDroneVisionMode UDroneHUDWidget::GetVisionMode() const
{
	if (VisionComponent)
	{
		return VisionComponent->GetVisionMode();
	}
	return EDroneVisionMode::Normal;
}

FString UDroneHUDWidget::GetVisionModeText() const
{
	EDroneVisionMode Mode = GetVisionMode();

	switch (Mode)
	{
	case EDroneVisionMode::Normal:
		return TEXT("NORMAL");
	case EDroneVisionMode::Night:
		return TEXT("NIGHT VISION");
	case EDroneVisionMode::Thermal:
		return TEXT("THERMAL");
	default:
		return TEXT("UNKNOWN");
	}
}

FLinearColor UDroneHUDWidget::GetVisionModeColor() const
{
	EDroneVisionMode Mode = GetVisionMode();

	switch (Mode)
	{
	case EDroneVisionMode::Normal:
		return FLinearColor::White;
	case EDroneVisionMode::Night:
		return FLinearColor::Green;
	case EDroneVisionMode::Thermal:
		return FLinearColor(1.0f, 0.5f, 0.0f); // Orange
	default:
		return FLinearColor::White;
	}
}

TArray<AActor*> UDroneHUDWidget::GetMarkedTargets() const
{
	if (MarkingComponent)
	{
		return MarkingComponent->GetMarkedTargets();
	}
	return TArray<AActor*>();
}

int32 UDroneHUDWidget::GetMarkedTargetCount() const
{
	return GetMarkedTargets().Num();
}

TArray<FThermalDetection> UDroneHUDWidget::GetThermalDetections() const
{
	if (VisionComponent)
	{
		return VisionComponent->GetThermalDetections();
	}
	return TArray<FThermalDetection>();
}

bool UDroneHUDWidget::HasTargetInCrosshair() const
{
	return GetTargetInCrosshair() != nullptr;
}

AActor* UDroneHUDWidget::GetTargetInCrosshair() const
{
	if (!OwningDrone)
		return nullptr;

	// Get camera
	UCameraComponent* Camera = OwningDrone->FindComponentByClass<UCameraComponent>();
	if (!Camera)
		return nullptr;

	// Perform line trace from camera
	FVector Start = Camera->GetComponentLocation();
	FVector Forward = Camera->GetForwardVector();
	FVector End = Start + (Forward * 5000.0f);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwningDrone);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		return Hit.GetActor();
	}

	return nullptr;
}

float UDroneHUDWidget::GetDistanceToTarget(AActor* Target) const
{
	if (!Target || !OwningDrone)
		return 0.0f;

	return FVector::Dist(OwningDrone->GetActorLocation(), Target->GetActorLocation());
}

bool UDroneHUDWidget::IsFlashlightOn() const
{
	if (UtilityComponent)
	{
		return UtilityComponent->IsFlashlightEnabled();
	}
	return false;
}

bool UDroneHUDWidget::GetScreenPositionForActor(AActor* Actor, FVector2D& OutScreenPosition) const
{
	if (!Actor)
		return false;

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
		return false;

	FVector ActorLocation = Actor->GetActorLocation();
	return PC->ProjectWorldLocationToScreen(ActorLocation, OutScreenPosition, false);
}

bool UDroneHUDWidget::IsActorOnScreen(AActor* Actor) const
{
	FVector2D ScreenPosition;
	if (!GetScreenPositionForActor(Actor, ScreenPosition))
		return false;

	// Check if within viewport bounds
	int32 ViewportSizeX, ViewportSizeY;
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		PC->GetViewportSize(ViewportSizeX, ViewportSizeY);
		return ScreenPosition.X >= 0 && ScreenPosition.X <= ViewportSizeX &&
		       ScreenPosition.Y >= 0 && ScreenPosition.Y <= ViewportSizeY;
	}

	return false;
}

void UDroneHUDWidget::CacheComponents()
{
	if (!OwningDrone)
		return;

	BatteryComponent = OwningDrone->GetDroneBattery();
	VisionComponent = OwningDrone->GetDroneVision();
	MarkingComponent = OwningDrone->GetDroneMarking();
	UtilityComponent = OwningDrone->GetDroneUtility();
}

FString UDroneHUDWidget::FormatDistance(float Distance) const
{
	if (Distance < 1000.0f)
	{
		return FString::Printf(TEXT("%.0fm"), Distance / 100.0f);
	}
	else
	{
		return FString::Printf(TEXT("%.1fkm"), Distance / 100000.0f);
	}
}
