// Fill out your copyright notice in the Description page of Project Settings.


#include "StoreBox.h"

#include "PlayerCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/HUD.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AStoreBox::AStoreBox()
{
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(GetRootComponent());
}

void AStoreBox::Use_Implementation(APlayerCharacter* Player)
{
	OpenStoreMenu();
}

void AStoreBox::ShowInteractable_Implementation(bool bShow)
{
	StaticMeshComponent->SetRenderCustomDepth(bShow);
}

void AStoreBox::OpenStoreMenu()
{
	if (BuyBoxWidgetClass)
	{
		if (UUserWidget* BuyBoxWidget = CreateWidget<UUserWidget>(GetWorld(), BuyBoxWidgetClass))
		{
			BuyBoxWidget->AddToViewport();
			TArray<UUserWidget*> FoundWidgets;
			UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UUserWidget::StaticClass(), false);

			for (UUserWidget* Widget : FoundWidgets)
			{
				if (Widget && Widget->IsInViewport() && Widget->GetClass() == HudWidgetClass)
				{
					Widget->RemoveFromParent();
					break;
				}
			}
			
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
