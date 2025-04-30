// Fill out your copyright notice in the Description page of Project Settings.


#include "StoreBox.h"

#include "Blueprint/UserWidget.h"
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
				// Show mouse cursor if needed
				PlayerController->bShowMouseCursor = true;
				PlayerController->SetInputMode(FInputModeUIOnly());
			}
		}
	}
}
