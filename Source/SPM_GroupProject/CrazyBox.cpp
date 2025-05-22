// Fill out your copyright notice in the Description page of Project Settings.


#include "CrazyBox.h"

#include "PlayerGameInstance.h"

// Sets default values
ACrazyBox::ACrazyBox()
{
	CubeMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("CubeMesh");
}

void ACrazyBox::Use_Implementation(APlayerCharacter* Player)
{
	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		if (bCrazy)
		{
			GI->NextDialogueRowName = "";
			bCrazy = false;
		}
		else
		{
			GI->StartDialogueRowName = "Crazy?";
			GI->StartDialogue();
			bCrazy = true;
		}
	}
}

void ACrazyBox::ShowInteractable_Implementation(bool bShow)
{
	CubeMeshComponent->SetRenderCustomDepth(bShow);
}

