// Fill out your copyright notice in the Description page of Project Settings.


#include "ArenaKillZVolume.h"
#include "Engine/DamageEvents.h"


void AArenaKillZVolume::ActorEnteredVolume(class AActor* Other)
{
	FDamageEvent DamageEvent;
	Other->TakeDamage(10000000, DamageEvent, nullptr, this);
}
