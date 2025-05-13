// Fill out your copyright notice in the Description page of Project Settings.


#include "TeleportScreen.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ATeleportScreen::ATeleportScreen()
{
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(GetRootComponent());
}

void ATeleportScreen::Use_Implementation(APlayerCharacter* Player)
{if (TeleportWidgetClass)
{
	if (UUserWidget* BuyBoxWidget = CreateWidget<UUserWidget>(GetWorld(), TeleportWidgetClass))
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
			
		if (OpenTeleportMenuSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), OpenTeleportMenuSound, GetActorLocation());
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

