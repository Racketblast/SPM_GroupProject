// Fill out your copyright notice in the Description page of Project Settings.


#include "BuyBox.h"

#include "PlayerGameInstance.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"



ABuyBox::ABuyBox()
{
	CubeMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CubeMeshComponent"));
	CubeMeshComponent->SetupAttachment(GetRootComponent());
}