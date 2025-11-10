// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

/**
 * Custom details panel for UDroneConfig
 * Provides improved editor experience for drone configuration
 */
class FDroneConfigCustomization : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	void CustomizeMovementCategory(IDetailLayoutBuilder& DetailBuilder);
	void CustomizeBatteryCategory(IDetailLayoutBuilder& DetailBuilder);
	void CustomizeSensorsCategory(IDetailLayoutBuilder& DetailBuilder);
	void CustomizeNetworkingCategory(IDetailLayoutBuilder& DetailBuilder);
};
