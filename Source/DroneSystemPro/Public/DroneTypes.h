// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DroneTypes.generated.h"

/**
 * Vision modes available to the drone
 */
UENUM(BlueprintType)
enum class EDroneVisionMode : uint8
{
	Normal		UMETA(DisplayName = "Normal Vision"),
	Night		UMETA(DisplayName = "Night Vision"),
	Thermal		UMETA(DisplayName = "Thermal Vision")
};

/**
 * Drone speed modes
 */
UENUM(BlueprintType)
enum class EDroneSpeedMode : uint8
{
	Low			UMETA(DisplayName = "Low Speed"),
	High		UMETA(DisplayName = "High Speed")
};

/**
 * Drone AI behavior types
 */
UENUM(BlueprintType)
enum class EDroneBehaviorType : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Patrol		UMETA(DisplayName = "Patrol"),
	Follow		UMETA(DisplayName = "Follow"),
	Scan		UMETA(DisplayName = "Scan Area"),
	AttackMark	UMETA(DisplayName = "Attack Mark")
};

/**
 * Marked target information
 */
USTRUCT(BlueprintType)
struct FMarkedTarget
{
	GENERATED_BODY()

	UPROPERTY()
	AActor* Target = nullptr;

	UPROPERTY()
	float MarkTime = 0.0f;

	UPROPERTY()
	float Duration = 5.0f;

	FMarkedTarget() {}

	FMarkedTarget(AActor* InTarget, float InDuration)
		: Target(InTarget), MarkTime(0.0f), Duration(InDuration)
	{}

	bool IsValid() const { return Target != nullptr; }
	bool IsExpired(float CurrentTime) const { return (CurrentTime - MarkTime) > Duration; }
};

/**
 * Thermal detection data for networked transmission
 */
USTRUCT(BlueprintType)
struct FThermalDetection
{
	GENERATED_BODY()

	UPROPERTY()
	AActor* DetectedActor = nullptr;

	UPROPERTY()
	FVector Location = FVector::ZeroVector;

	UPROPERTY()
	float HeatSignature = 0.0f;

	FThermalDetection() {}

	FThermalDetection(AActor* InActor, FVector InLocation, float InHeat)
		: DetectedActor(InActor), Location(InLocation), HeatSignature(InHeat)
	{}
};

/**
 * Movement snapshot for client prediction
 */
USTRUCT()
struct FDroneMovementSnapshot
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location = FVector::ZeroVector;

	UPROPERTY()
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY()
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY()
	float Timestamp = 0.0f;

	UPROPERTY()
	uint32 InputID = 0;

	FDroneMovementSnapshot() {}

	FDroneMovementSnapshot(FVector InLoc, FRotator InRot, FVector InVel, float InTime, uint32 InID)
		: Location(InLoc), Rotation(InRot), Velocity(InVel), Timestamp(InTime), InputID(InID)
	{}
};

/**
 * Client input data for replication
 */
USTRUCT()
struct FDroneInputState
{
	GENERATED_BODY()

	UPROPERTY()
	FVector MovementInput = FVector::ZeroVector;

	UPROPERTY()
	FVector2D LookInput = FVector2D::ZeroVector;

	UPROPERTY()
	float DeltaTime = 0.0f;

	UPROPERTY()
	uint32 InputID = 0;

	UPROPERTY()
	float Timestamp = 0.0f;

	FDroneInputState() {}
};

/**
 * Hacking session state
 */
USTRUCT(BlueprintType)
struct FHackingSession
{
	GENERATED_BODY()

	UPROPERTY()
	AActor* HackerActor = nullptr;

	UPROPERTY()
	AActor* TargetActor = nullptr;

	UPROPERTY()
	float Progress = 0.0f;

	UPROPERTY()
	float Duration = 5.0f;

	UPROPERTY()
	float StartTime = 0.0f;

	UPROPERTY()
	bool bIsActive = false;

	FHackingSession() {}
};

/**
 * Drone configuration DataAsset
 * Defines all drone stats and parameters
 */
UCLASS(BlueprintType)
class DRONESYSTEMPRO_API UDroneConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	// Movement
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float MaxSpeedLow = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float MaxSpeedHigh = 1200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float Acceleration = 1000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float Deceleration = 2000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float TurnRate = 180.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float MaxPitchAngle = 45.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float MaxRollAngle = 45.0f;

	// Battery
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Battery")
	float MaxBattery = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Battery")
	float BatteryDrainIdle = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Battery")
	float BatteryDrainLowSpeed = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Battery")
	float BatteryDrainHighSpeed = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Battery")
	float BatteryDrainFlashlight = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Battery")
	float BatteryDrainNightVision = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Battery")
	float BatteryDrainThermalVision = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Battery")
	float BatteryDrainScanning = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Battery")
	float BatteryRechargeRate = 5.0f;

	// Sensors
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sensors")
	float SensorRange = 3000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sensors")
	float ThermalDetectionRange = 2000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sensors")
	float MarkingRange = 2500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sensors")
	float MarkDuration = 10.0f;

	// Networking
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Networking")
	float ReplicationRate = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Networking")
	float NetCullDistance = 15000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Networking")
	float RelevancyCheckInterval = 0.5f;
};

/**
 * Drone behavior configuration DataAsset
 */
UCLASS(BlueprintType)
class DRONESYSTEMPRO_API UDroneBehaviorProfile : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
	EDroneBehaviorType BehaviorType = EDroneBehaviorType::Idle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
	float PatrolRadius = 1000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
	float FollowDistance = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
	float ScanRadius = 1500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
	float ScanDuration = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
	float AggressionLevel = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
	bool bAutoMarkEnemies = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
	float ReactionTime = 0.5f;
};
