// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GAS : ModuleRules
{
	public GAS(ReadOnlyTargetRules Target) : base(Target)
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
			"Slate", "GameplayAbilities"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"GameplayAbilities",
			"GameplayTasks",
			"GameplayTags"
		});

		PublicIncludePaths.AddRange(new string[] {
			"GAS",
			"GAS/Variant_Platforming",
			"GAS/Variant_Platforming/Animation",
			"GAS/Variant_Combat",
			"GAS/Variant_Combat/AI",
			"GAS/Variant_Combat/Animation",
			"GAS/Variant_Combat/Gameplay",
			"GAS/Variant_Combat/Interfaces",
			"GAS/Variant_Combat/UI",
			"GAS/Variant_SideScrolling",
			"GAS/Variant_SideScrolling/AI",
			"GAS/Variant_SideScrolling/Gameplay",
			"GAS/Variant_SideScrolling/Interfaces",
			"GAS/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
