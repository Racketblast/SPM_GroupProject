// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ImpactExplosive.h"
#include "EndGameExplosive.generated.h"

/**
 * 
 */
UCLASS()
class SPM_GROUPPROJECT_API AEndGameExplosive : public AImpactExplosive
{
	GENERATED_BODY()
	virtual void Explode() override;

public:
	UPROPERTY(BlueprintReadOnly)
	class ULevelSequencePlayer* SequencePlayer;
	
	UPROPERTY(EditDefaultsOnly, Category="Transition")
	class ULevelSequence* FadeOutTransition;
};
