// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DroneSystemProEditor : ModuleRules
{
	public DroneSystemProEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"DroneSystemPro"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"UnrealEd",
				"Slate",
				"SlateCore",
				"PropertyEditor",
				"EditorStyle",
				"AssetTools",
				"ContentBrowser"
			}
		);
	}
}
