// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SPM_GroupProject : ModuleRules
{
	public SPM_GroupProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "MovieScene", "MovieSceneTracks", "LevelSequence", "UMG", "AIModule", "GameplayTasks","NavigationSystem"});
	}
}
