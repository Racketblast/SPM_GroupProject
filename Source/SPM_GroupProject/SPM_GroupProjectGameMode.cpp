// Copyright Epic Games, Inc. All Rights Reserved.

#include "SPM_GroupProjectGameMode.h"
#include "SPM_GroupProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASPM_GroupProjectGameMode::ASPM_GroupProjectGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
