// Fill out your copyright notice in the Description page of Project Settings.


#include "Teleporter.h"

// Sets default values
ATeleporter::ATeleporter()
{
	CubeMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CubeMeshComponent"));
	CubeMeshComponent->SetupAttachment(GetRootComponent());
}

