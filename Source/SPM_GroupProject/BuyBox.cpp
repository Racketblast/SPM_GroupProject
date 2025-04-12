// Fill out your copyright notice in the Description page of Project Settings.


#include "BuyBox.h"

#include "PlayerStats.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABuyBox::ABuyBox()
{
	BoxMeshComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxMeshComponent"));
	RootComponent = BoxMeshComponent;
}


