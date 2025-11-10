// Copyright Epic Games, Inc. All Rights Reserved.

#include "DroneConfigCustomization.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "DroneConfigCustomization"

TSharedRef<IDetailCustomization> FDroneConfigCustomization::MakeInstance()
{
	return MakeShareable(new FDroneConfigCustomization());
}

void FDroneConfigCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Customize categories
	CustomizeMovementCategory(DetailBuilder);
	CustomizeBatteryCategory(DetailBuilder);
	CustomizeSensorsCategory(DetailBuilder);
	CustomizeNetworkingCategory(DetailBuilder);
}

void FDroneConfigCustomization::CustomizeMovementCategory(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Movement", FText::GetEmpty(), ECategoryPriority::Important);

	// Add header text
	Category.AddCustomRow(FText::GetEmpty())
		.WholeRowContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("MovementHeader", "Configure drone movement parameters including speed, acceleration, and rotation limits."))
			.AutoWrapText(true)
		];
}

void FDroneConfigCustomization::CustomizeBatteryCategory(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Battery", FText::GetEmpty(), ECategoryPriority::Important);

	// Add header text
	Category.AddCustomRow(FText::GetEmpty())
		.WholeRowContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("BatteryHeader", "Configure battery capacity and drain rates for different features."))
			.AutoWrapText(true)
		];
}

void FDroneConfigCustomization::CustomizeSensorsCategory(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Sensors", FText::GetEmpty(), ECategoryPriority::Important);

	// Add header text
	Category.AddCustomRow(FText::GetEmpty())
		.WholeRowContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SensorsHeader", "Configure sensor ranges for vision, thermal detection, and marking."))
			.AutoWrapText(true)
		];
}

void FDroneConfigCustomization::CustomizeNetworkingCategory(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Networking", FText::GetEmpty(), ECategoryPriority::Important);

	// Add header text
	Category.AddCustomRow(FText::GetEmpty())
		.WholeRowContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("NetworkingHeader", "Configure network replication settings for optimal multiplayer performance."))
			.AutoWrapText(true)
		];
}

#undef LOCTEXT_NAMESPACE
