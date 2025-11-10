// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * DroneSystemPro Editor Module
 * Provides editor tools for DataAsset creation and custom details panels
 */
class FDroneSystemProEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterAssetTypeActions();
	void UnregisterAssetTypeActions();
	void RegisterCustomDetails();
	void UnregisterCustomDetails();

	TArray<TSharedPtr<class IAssetTypeActions>> RegisteredAssetTypeActions;
};
