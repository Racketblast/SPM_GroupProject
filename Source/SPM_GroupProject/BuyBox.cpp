// Fill out your copyright notice in the Description page of Project Settings.


#include "BuyBox.h"

#include "PlayerGameInstance.h"


ABuyBox::ABuyBox()
{
	CubeMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CubeMeshComponent"));
	CubeMeshComponent->SetupAttachment(GetRootComponent());
}

void ABuyBox::Use_Implementation(APlayerCharacter* Player)
{
	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		GI->BuyUpgrade(TargetUpgradeName, BuySound, CantBuySound);
	}
}
