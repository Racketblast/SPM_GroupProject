// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SPM_GroupProject : ModuleRules
{
	public SPM_GroupProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Chaos","Core", "ProceduralMeshComponent",  "GeometryCollectionEngine",
			"CoreUObject", "Engine", "InputCore", "EnhancedInput", "MovieScene", "MovieSceneTracks", "LevelSequence", "UMG", "Niagara", "AIModule", "GameplayTasks","NavigationSystem", "GeometryCollectionEngine","ProceduralMeshComponent","ChaosSolverEngine"});
	}
}
