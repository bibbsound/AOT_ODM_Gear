// Copyright Epic Games, Inc. All Rights Reserved.

#include "AOT_ODM_GearGameMode.h"
#include "AOT_ODM_GearCharacter.h"
#include "UObject/ConstructorHelpers.h"

AAOT_ODM_GearGameMode::AAOT_ODM_GearGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
