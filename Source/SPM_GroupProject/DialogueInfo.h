// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DialogueInfo.generated.h"

USTRUCT(BlueprintType)
struct FDialogueInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USoundBase* DialogueSound = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<UUserWidget> DialogueWidgetClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bPlayOneTime = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bHasBeenPlayed = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString DialogueText = "";

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName NextDialogue = "";
	
	FDialogueInfo() {}

	FDialogueInfo(USoundBase* InSound, TSubclassOf<UUserWidget> InWidgetClass, bool bPlayed = false, FString InText = "", FName InNextDialogue = "")
		: DialogueSound(InSound), DialogueWidgetClass(InWidgetClass), bHasBeenPlayed(bPlayed), DialogueText(InText), NextDialogue(InNextDialogue)
	{}
};