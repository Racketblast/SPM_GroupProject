// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelLoader.h"

#include "PlayerGameInstance.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ALevelLoader::ALevelLoader()
{
}

// Called when the game starts or when spawned
void ALevelLoader::BeginPlay()
{
	Super::BeginPlay();
	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
	{
		for (UWorld* Level : Levels)
		{
			if (!GI->LevelOrder.Contains(FName(*Level->GetName())))
			{
				GI->LevelOrder.Add(FName(*Level->GetName()));
			}
		}
		if (GI->LevelOrder.Num() > 0)
		{
			GI->UnlockedLevels.Add(GI->LevelOrder[0]);
			GI->UnlockedLevels.Add(GI->LevelOrder[1]);
		}
	}
}


