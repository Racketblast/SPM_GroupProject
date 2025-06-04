// Fill out your copyright notice in the Description page of Project Settings.


#include "Teleporter.h"

#include "ArenaGameMode.h"
#include "LevelSequencePlayer.h"
#include "PlayerGameInstance.h"
#include "PlayerCharacter.h"
#include "MissionSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/BoxComponent.h"

// Sets default values
ATeleporter::ATeleporter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	CubeMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CubeMeshComponent"));
	TeleportTriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("PortalDistanceTriggerVolume"));
	TeleportTriggerVolume->SetupAttachment(CubeMeshComponent);

	TeleportSkyBeam = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TeleportSkyBeam"));
	TeleportSkyBeam->SetupAttachment(CubeMeshComponent);
	TeleportSkyBeam->bAutoActivate = false;

	TeleportCircles = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TeleportCircles"));
	TeleportCircles->SetupAttachment(CubeMeshComponent);
	TeleportCircles->bAutoActivate = false;
	
	if (TeleportTriggerVolume)
	{
		TeleportTriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ATeleporter::OnTriggerBeginOverlap);
	}

	InteractText = TEXT("Teleport");
}

void ATeleporter::Use_Implementation(APlayerCharacter* Player)
{
	Teleport();
}

void ATeleporter::ShowInteractable_Implementation(bool bShow)
{
	if (!CachedGameInstance->bIsWave)
		CubeMeshComponent->SetRenderCustomDepth(bShow);

	
	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		GI->CurrentInteractText = InteractText;
	}
}


void ATeleporter::BeginPlay()
{
	Super::BeginPlay();
	
	CachedGameInstance = Cast<UPlayerGameInstance>(GetGameInstance());
	
	if (CachedGameInstance)
	{
		bOldWaveValue = CachedGameInstance->bIsWave;
	}
	else
	{
		bOldWaveValue = false;
	}
	
	ChangeTexture();
}

void ATeleporter::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == UGameplayStatics::GetPlayerCharacter(GetWorld(),0))
	{
		if (TeleportWidgetClass && !CachedGameInstance->bIsWave && CachedGameInstance->UnlockedLevels.Contains(TargetLevelName))
		{
			if (UUserWidget* TeleportWidget = CreateWidget<UUserWidget>(GetWorld(), TeleportWidgetClass))
			{
				TeleportWidget->AddToViewport();
			}
		}
	}
}

void ATeleporter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!CachedGameInstance)
		return;

	if (bOldWaveValue == CachedGameInstance->bIsWave)
		return;

	bOldWaveValue = CachedGameInstance->bIsWave;
	ChangeTexture();
}

void ATeleporter::ChangeTexture()
{
	bool bHasAccess = CachedGameInstance->UnlockedLevels.Contains(TargetLevelName);
	//if it is a wave or if player does not have access to the level
	if (bOldWaveValue || !bHasAccess)
	{
		if (WaveMaterial)
		{
			if (CubeMeshComponent->GetMaterial(1))
			{
				CubeMeshComponent->SetMaterial(1, WaveMaterial);
			}
			CubeMeshComponent->SetRenderCustomDepth(false);
		}

		// Turn off effect if access denied
		if (TeleportSkyBeam && UGameplayStatics::GetCurrentLevelName(this,true) != TEXT("Hub"))
		{
			TeleportSkyBeam->Deactivate();
		}
		if (TeleportCircles && UGameplayStatics::GetCurrentLevelName(this,true) != TEXT("Hub"))
		{
			TeleportCircles->Deactivate();
		}
	}
	else
	{
		if (GracePeriodMaterial)
		{
			if (CubeMeshComponent->GetMaterial(1))
			{
				CubeMeshComponent->SetMaterial(1, GracePeriodMaterial);
			}
			CubeMeshComponent->SetRenderCustomDepth(true);
		}
		
		// Turn on effect if access denied
		if (TeleportSkyBeam && UGameplayStatics::GetCurrentLevelName(this,true) != TEXT("Hub"))
		{
			TeleportSkyBeam->Activate();
		}
		if (TeleportCircles && UGameplayStatics::GetCurrentLevelName(this,true) != TEXT("Hub"))
		{
			TeleportCircles->Activate();
		}
	}
}

void ATeleporter::Teleport()
{
	if (!CachedGameInstance)
		return;
		// If you can teleport
		// Checks if in wave and if you have level unlocked
		if (!CachedGameInstance->bIsWave && CachedGameInstance->UnlockedLevels.Contains(TargetLevelName))
		{
			if (APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
			{
				CachedGameInstance->Money += Player->PickedUpMoney;
			}
			
			if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
			{
				//Plays the mission incomplete dialogue for return
				if ( TargetLevelName == "Hub")
				{
					GI->StartDialogueRowName = "ReturnMissionIncomplete";
				}
				
				// För level unlock 
				if (UMissionSubsystem* MissionSub = CachedGameInstance->GetSubsystem<UMissionSubsystem>())
				{
					MissionSub->TryUnlockLevel();
					if (MissionSub->WavesSurvived >= 2 && GI->CurrentGameFlag < 2 && UGameplayStatics::GetCurrentLevelName(GetWorld(),true) == TEXT("V3"))
					{
						GI->CurrentGameFlag = 2;
						GI->StartDialogueRowName = "ReturnMissionComplete";
					}
					
					if (MissionSub->IsMissionCompleted())
					{
						//Plays the mission complete dialogue for return if mission is complete and updates the game flag
						if (UGameplayStatics::GetCurrentLevelName(GetWorld(),true) == TEXT("V3"))
						{
							if (GI->CurrentGameFlag < 3)
							{
								GI->CurrentGameFlag = 3;
							}
							GI->StartDialogueRowName = "ReturnMissionComplete";
						}
						else if (UGameplayStatics::GetCurrentLevelName(GetWorld(),true) == TEXT("MetroV3"))
						{
							if (GI->CurrentGameFlag < 4)
							{
								GI->CurrentGameFlag = 4;
							}
							GI->StartDialogueRowName = "EndGameGun1";
						}
					}
				}
			}
			
			if (AArenaGameMode* GameMode = Cast<AArenaGameMode>(UGameplayStatics::GetGameMode(this)))
			{
				GameMode->FadeOut(this);
				if (GameMode->SequencePlayer)
				{
					GameMode->SequencePlayer->OnFinished.AddDynamic(this, &ATeleporter::ChangeLevel);
				}
			}
		}
		else
		{
			if (CantTeleportSound)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), CantTeleportSound, GetActorLocation());
			}
		}
}

void ATeleporter::ChangeLevel()
{
	UGameplayStatics::OpenLevel(this, TargetLevelName);
}
