// Fill out your copyright notice in the Description page of Project Settings.


#include "StoreBox.h"

#include "PlayerCharacter.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AStoreBox::AStoreBox()
{
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(GetRootComponent());
}

void AStoreBox::OpenStoreMenu()
{
	if (BuyBoxWidgetClass)
	{
		if (UUserWidget* BuyBoxWidget = CreateWidget<UUserWidget>(GetWorld(), BuyBoxWidgetClass))
		{
			BuyBoxWidget->AddToViewport();
			
			if (OpenBuyMenuSound)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), OpenBuyMenuSound, GetActorLocation());
			}
			
			if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
			{
				PlayerController->SetIgnoreMoveInput(true);
				PlayerController->SetIgnoreLookInput(true);
				
				PlayerController->bShowMouseCursor = true;
				int32 ViewportX, ViewportY;
				PlayerController->GetViewportSize(ViewportX, ViewportY);
				PlayerController->SetMouseLocation(ViewportX/2, ViewportY/2);
				PlayerController->SetInputMode(FInputModeUIOnly());
			}
		}
	}
}
