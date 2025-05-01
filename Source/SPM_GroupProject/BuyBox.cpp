// Fill out your copyright notice in the Description page of Project Settings.


#include "BuyBox.h"



ABuyBox::ABuyBox()
{
	CubeMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CubeMeshComponent"));
	CubeMeshComponent->SetupAttachment(GetRootComponent());
}