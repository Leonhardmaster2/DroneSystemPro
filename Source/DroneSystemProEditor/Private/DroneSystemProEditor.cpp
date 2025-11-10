// Copyright Epic Games, Inc. All Rights Reserved.

#include "DroneSystemProEditor.h"
#include "AssetToolsModule.h"
#include "PropertyEditorModule.h"
#include "DroneConfigCustomization.h"
#include "DroneTypes.h"

#define LOCTEXT_NAMESPACE "FDroneSystemProEditorModule"

void FDroneSystemProEditorModule::StartupModule()
{
	// Register asset type actions
	RegisterAssetTypeActions();

	// Register custom details panels
	RegisterCustomDetails();
}

void FDroneSystemProEditorModule::ShutdownModule()
{
	// Unregister asset type actions
	UnregisterAssetTypeActions();

	// Unregister custom details panels
	UnregisterCustomDetails();
}

void FDroneSystemProEditorModule::RegisterAssetTypeActions()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// Register DataAsset factories here if needed
	// For now, UE's default DataAsset handling is sufficient
}

void FDroneSystemProEditorModule::UnregisterAssetTypeActions()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();

		for (auto& Action : RegisteredAssetTypeActions)
		{
			AssetTools.UnregisterAssetTypeActions(Action.ToSharedRef());
		}
	}

	RegisteredAssetTypeActions.Empty();
}

void FDroneSystemProEditorModule::RegisterCustomDetails()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	// Register custom details for UDroneConfig
	PropertyModule.RegisterCustomClassLayout(
		UDroneConfig::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FDroneConfigCustomization::MakeInstance)
	);
}

void FDroneSystemProEditorModule::UnregisterCustomDetails()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

		PropertyModule.UnregisterCustomClassLayout(UDroneConfig::StaticClass()->GetFName());
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDroneSystemProEditorModule, DroneSystemProEditor)
