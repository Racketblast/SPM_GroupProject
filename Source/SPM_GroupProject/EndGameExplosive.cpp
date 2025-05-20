// Fill out your copyright notice in the Description page of Project Settings.


#include "EndGameExplosive.h"

void AEndGameExplosive::Explode()
{
	UE_LOG(LogTemp, Warning, TEXT("Exploded"));
	Destroy();
}
