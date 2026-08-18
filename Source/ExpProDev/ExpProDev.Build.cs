// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ExpProDev : ModuleRules
{
	public ExpProDev(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// TECH_DEBT(TD-CFG-1): several of these have no C++ usage in the module and only inflate
		// build and link times:
		//   StateTreeModule / GameplayStateTreeModule — zero references anywhere in Source/.
		//   OnlineSubsystem / OnlineSubsystemSteam    — no C++ usage; Steam is plugin/config driven.
		//   Json / JsonUtilities                      — used only by the dead WebSockets/ folder.
		// Remove each as its dependent dead code is deleted (TD-DEAD-1, TD-DEAD-8).
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"OnlineSubsystemSteam",
			"OnlineSubsystem",
			"Json",
			"JsonUtilities"
		});

		// TECH_DEBT(TD-CFG-1): WebSockets serves only the dead prototype (TD-DEAD-1);
		// ProceduralMeshComponent serves only AProcEnvironmentGenerator (TD-DEAD-10).
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"WebSockets",
			"ProceduralMeshComponent"
		});

		// TECH_DEBT(TD-CFG-2): the six Variant_* entries exist so template code can use flat
		// includes. They also let any new file accidentally include variant headers without a path,
		// which is how the dead variants keep getting pulled back into the build. Remove with
		// TD-DEAD-7 / TD-DEAD-8.
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
