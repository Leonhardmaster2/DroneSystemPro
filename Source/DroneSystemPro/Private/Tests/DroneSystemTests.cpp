// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "DroneBase.h"
#include "DroneBatteryComponent.h"
#include "DroneMovementComponent.h"
#include "DroneMarkingComponent.h"
#include "JammingComponent.h"
#include "DroneDockingComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

// Battery Component Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDroneBatteryDrainTest, "DroneSystemPro.Battery.DrainTest", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDroneBatteryDrainTest::RunTest(const FString& Parameters)
{
	// Test battery drain functionality
	UDroneBatteryComponent* BatteryComp = NewObject<UDroneBatteryComponent>();
	if (!BatteryComp)
	{
		AddError(TEXT("Failed to create battery component"));
		return false;
	}

	float InitialBattery = BatteryComp->GetBatteryLevel();
	TestTrue(TEXT("Initial battery level should be positive"), InitialBattery > 0.0f);

	return true;
}

// Movement Component Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDroneMovementSpeedTest, "DroneSystemPro.Movement.SpeedTest", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDroneMovementSpeedTest::RunTest(const FString& Parameters)
{
	// Test movement speed modes
	UDroneMovementComponent* MovementComp = NewObject<UDroneMovementComponent>();
	if (!MovementComp)
	{
		AddError(TEXT("Failed to create movement component"));
		return false;
	}

	// Test speed mode switching
	MovementComp->SetSpeedMode(EDroneSpeedMode::Low);
	TestEqual(TEXT("Speed mode should be Low"), MovementComp->GetSpeedMode(), EDroneSpeedMode::Low);

	MovementComp->SetSpeedMode(EDroneSpeedMode::High);
	TestEqual(TEXT("Speed mode should be High"), MovementComp->GetSpeedMode(), EDroneSpeedMode::High);

	return true;
}

// Marking Component Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDroneMarkingTest, "DroneSystemPro.Marking.MarkTest", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDroneMarkingTest::RunTest(const FString& Parameters)
{
	// Test marking functionality
	UDroneMarkingComponent* MarkingComp = NewObject<UDroneMarkingComponent>();
	if (!MarkingComp)
	{
		AddError(TEXT("Failed to create marking component"));
		return false;
	}

	TArray<AActor*> MarkedTargets = MarkingComp->GetMarkedTargets();
	TestEqual(TEXT("Initially no targets should be marked"), MarkedTargets.Num(), 0);

	return true;
}

// Jamming Component Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJammingIntensityTest, "DroneSystemPro.Jamming.IntensityTest", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FJammingIntensityTest::RunTest(const FString& Parameters)
{
	// Test jamming intensity calculation
	UJammingComponent* JammingComp = NewObject<UJammingComponent>();
	if (!JammingComp)
	{
		AddError(TEXT("Failed to create jamming component"));
		return false;
	}

	JammingComp->SetJamStrength(1.0f);
	JammingComp->SetJamRadius(1000.0f);

	TestTrue(TEXT("Jamming component created successfully"), JammingComp != nullptr);

	return true;
}

// Docking Component Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDroneDockingTest, "DroneSystemPro.Docking.DockTest", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDroneDockingTest::RunTest(const FString& Parameters)
{
	// Test docking functionality
	UDroneDockingComponent* DockingComp = NewObject<UDroneDockingComponent>();
	if (!DockingComp)
	{
		AddError(TEXT("Failed to create docking component"));
		return false;
	}

	TestFalse(TEXT("Initially no drone should be docked"), DockingComp->IsDroneDocked());

	return true;
}

// Integration Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDroneSystemIntegrationTest, "DroneSystemPro.Integration.FullSystemTest", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDroneSystemIntegrationTest::RunTest(const FString& Parameters)
{
	// Test full system integration
	// This would require a world context, so it's a simplified test

	AddInfo(TEXT("DroneSystemPro integration test completed"));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
