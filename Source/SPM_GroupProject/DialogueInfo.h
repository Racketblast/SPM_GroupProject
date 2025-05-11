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
	int32 DialogueFlag = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString DialogueText = "";

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName NextDialogue = "";
	
	FDialogueInfo() {}

	FDialogueInfo(USoundBase* InSound, TSubclassOf<UUserWidget> InWidgetClass, int32 InDialogueFlag = 0, FString InText = "", FName InNextDialogue = "")
		: DialogueSound(InSound), DialogueWidgetClass(InWidgetClass), DialogueFlag(InDialogueFlag), DialogueText(InText), NextDialogue(InNextDialogue)
	{}
};