#include "Gun.h"
#include "PlayerCharacter.h"
#include "PlayerGameInstance.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AGun::AGun()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMeshComponent);

	MuzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePoint"));
	MuzzlePoint->SetupAttachment(WeaponMeshComponent);
	WeaponSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponSkeletalMesh"));
	WeaponSkeletalMesh->SetupAttachment(WeaponMeshComponent);
	MuzzlePoint->SetRelativeLocation(FVector(50.f, 0.f, 0.f));
}

void AGun::BeginPlay()
{
	Super::BeginPlay();
	BaseWeaponDamage = WeaponDamage;
	BaseRoundsPerSecond = RoundsPerSecond;
	BaseTotalAmmo = TotalAmmo;
	BaseMaxAmmo = MaxAmmo;
	BaseMaxTotalAmmo = MaxTotalAmmo;

	// 🔊 Initialize reload audio component
	if (!ReloadAudioComponent)
	{
		ReloadAudioComponent = NewObject<UAudioComponent>(this, TEXT("ReloadAudioComponent"));
		if (ReloadAudioComponent)
		{
			ReloadAudioComponent->RegisterComponent();
			ReloadAudioComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		}
	}
}

void AGun::Reload()
{
	if (bIsReloading)
	{
		UE_LOG(LogTemp, Warning, TEXT("Reload already in progress!"));
		return;
	}

	if (CurrentAmmo == MaxAmmo)
	{
		UE_LOG(LogTemp, Warning, TEXT("Ammo already full."));
		return;
	}

	if (!bHasInfiniteReloads && TotalAmmo <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No extra mags left! Reload cannot be performed."));
		return;
	}

	bIsReloading = true;

	UE_LOG(LogTemp, Warning, TEXT("Reload started..."));

	// 🔊 Play reload sound
	if (ReloadSound && ReloadAudioComponent)
	{
		if (ReloadAudioComponent->IsPlaying())
		{
			ReloadAudioComponent->Stop();
		}
		ReloadAudioComponent->SetSound(ReloadSound);
		ReloadAudioComponent->Play();
	}

	// Start a 2-second timer to finish the reload
	GetWorld()->GetTimerManager().SetTimer(ReloadTimerHandle, this, &AGun::FinishReload, 2.0f, false);
}

void AGun::FinishReload()
{
	if (bHasInfiniteReloads)
	{
		CurrentAmmo = MaxAmmo;
	}
	else
	{
		int missingAmmo = MaxAmmo - CurrentAmmo;
		if (missingAmmo <= TotalAmmo)
		{
			CurrentAmmo = MaxAmmo;
			TotalAmmo -= missingAmmo;
		}
		else
		{
			CurrentAmmo += TotalAmmo;
			TotalAmmo = 0;
		}
	}

	bIsReloading = false;

	UE_LOG(LogTemp, Warning, TEXT("Reload complete. CurrentAmmo: %d, TotalAmmo: %d"), CurrentAmmo, TotalAmmo);
}

void AGun::SetOwnerCharacter(APlayerCharacter* NewOwner)
{
	OwnerCharacter = NewOwner;
}

void AGun::CheckForUpgrades()
{
	if (bHasAppliedUpgrades) return;

	if (!bIsUpgraded)
	{
		if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
		{
			if (APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0)))
			{
				for (const TPair<EUpgradeType, FUpgradeInfo>& Upgrade : GI->UpgradeMap)
				{
					GI->UpgradeGunStats(Upgrade.Key, Player);
				}
			}
		}
		bHasAppliedUpgrades = true;
	}
}
