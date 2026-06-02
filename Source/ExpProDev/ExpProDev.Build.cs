// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ExpProDev : ModuleRules
{
	public ExpProDev(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"OnlineSubsystemSteam",
			"OnlineSubsystem",
			"Json",
			"JsonUtilities"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"WebSockets",
			"ProceduralMeshComponent"
		});

		PublicIncludePaths.AddRange(new string[] {
			"ExpProDev",
			"ExpProDev/Variant_Horror",
			"ExpProDev/Variant_Horror/UI",
			"ExpProDev/Variant_Shooter",
			"ExpProDev/Variant_Shooter/AI",
			"ExpProDev/Variant_Shooter/UI",
			"ExpProDev/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
