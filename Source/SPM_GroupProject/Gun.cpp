#include "Gun.h"

#include "FlyingEnemyAI.h"
#include "PlayerCharacter.h"
#include "PlayerGameInstance.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Components/DecalComponent.h"

AGun::AGun()
{
	PrimaryActorTick.bCanEverTick = true;
	WeaponMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMeshComponent);

	MuzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePoint"));
	MuzzlePoint->SetupAttachment(WeaponMeshComponent);
	WeaponSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponSkeletalMesh"));
	WeaponSkeletalMesh->SetupAttachment(WeaponMeshComponent);
	MuzzlePoint->SetRelativeLocation(FVector(50.f, 0.f, 0.f));

	//Sound setup
	FireAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("FireAudioComponent"));
	FireAudioComponent->SetupAttachment(WeaponMeshComponent);
}

void AGun::BeginPlay()
{
	Super::BeginPlay();
	if (!FireAudioComponent)
	{
		FireAudioComponent = NewObject<UAudioComponent>(this, TEXT("FireAudioComponent"));
		if (FireAudioComponent)
		{
			FireAudioComponent->RegisterComponent();
			FireAudioComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		}
	}
	if (!MagEmptyAudioComponent)
	{
		MagEmptyAudioComponent = NewObject<UAudioComponent>(this, TEXT("MagEmptyAudioComponent"));
		if (MagEmptyAudioComponent)
		{
			MagEmptyAudioComponent->RegisterComponent();
			MagEmptyAudioComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		}
	}
	BaseWeaponDamage = WeaponDamage;
	BaseRoundsPerSecond = RoundsPerSecond;
	BaseTotalAmmo = TotalAmmo;
	BaseMaxAmmo = MaxAmmo;
	BaseMaxTotalAmmo = MaxTotalAmmo;
	
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
void AGun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PrimaryActorTick.bCanEverTick = true;

	if (bIsRecoveringFromRecoil)
	{
		RecoilRecoveryElapsed += DeltaTime;
		float Alpha = FMath::Clamp(RecoilRecoveryElapsed / RecoilRecoveryDuration, 0.0f, 1.0f);

		APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OwnerCharacter);
		if (PlayerCharacter && PlayerCharacter->ArmsRoot)
		{
			FVector NewLocation = FMath::Lerp(RecoilStartLocation, RecoilTargetLocation, Alpha);
			PlayerCharacter->ArmsRoot->SetRelativeLocation(NewLocation);
		}

		if (Alpha >= 1.0f)
		{
			bIsRecoveringFromRecoil = false;
			bRecoilApplied = false;
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

	
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0)))
	{
		Player->bCanSwitchWeapons = false;
	}
	

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
	
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0)))
	{
		Player->bCanSwitchWeapons = true;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Reload complete. CurrentAmmo: %d, TotalAmmo: %d"), CurrentAmmo, TotalAmmo);
}

void AGun::SetOwnerCharacter(APlayerCharacter* NewOwner)
{
	OwnerCharacter = NewOwner;
}


void AGun::ApplyBloodDecal(const FHitResult& Hit)
{
	if (!BloodDecalMaterial) return;

	AActor* HitActor = Hit.GetActor();
	if (!HitActor || HitActor->IsA(AFlyingEnemyAI::StaticClass())) return;

	USkeletalMeshComponent* SkeletalMesh = HitActor->FindComponentByClass<USkeletalMeshComponent>();
	if (!SkeletalMesh) return;

	// === Bone-attached blood decal cluster ===
	const int32 ClusterCount = 4;
	const float ClusterRadius = 3.0f;
	const float ClusterDecalScale = decalSize * 0.5f;

	FRotator DecalRotation = Hit.Normal.Rotation();
	FName BoneName = Hit.BoneName != NAME_None ? Hit.BoneName : NAME_None;

	for (int32 i = 0; i < ClusterCount; ++i)
	{
		FVector RandomOffset = FMath::VRand() * ClusterRadius;
		FVector Location = Hit.ImpactPoint + RandomOffset;
		FRotator Rotation = DecalRotation;
		Rotation.Yaw += FMath::RandRange(-20.f, 20.f);

		UDecalComponent* BloodDecal = UGameplayStatics::SpawnDecalAttached(
			BloodDecalMaterial,
			FVector(ClusterDecalScale),
			SkeletalMesh,
			BoneName,
			Location,
			Rotation,
			EAttachLocation::KeepWorldPosition,
			20.0f
		);

		if (BloodDecal)
		{
			BloodDecal->SetFadeScreenSize(0.001f);
		}
	}

	// === Spawn Nearby Blood Splatter on Environment ===
	const int32 NumSplatterDecals = 2;
	const float MaxDistance = 125.f;
	const float SurfaceOffset = 0.5f;

	for (int32 i = 0; i < NumSplatterDecals; ++i)
	{
		FVector RandomDirection = FMath::VRand();
		FVector Start = Hit.ImpactPoint + RandomDirection * 20.f;
		FVector End = Start + RandomDirection * MaxDistance;

		FHitResult SurfaceHit;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		QueryParams.AddIgnoredActor(HitActor);
		QueryParams.bReturnPhysicalMaterial = false;
		QueryParams.bTraceComplex = true;

		if (GetWorld()->LineTraceSingleByChannel(SurfaceHit, Start, End, ECC_Visibility, QueryParams))
		{
			// Avoid splatter hitting the character
			if (SurfaceHit.GetActor() == HitActor) continue;

			FRotator SurfaceRotation = SurfaceHit.Normal.Rotation();
			SurfaceRotation.Yaw += FMath::RandRange(-180.f, 180.f);

			FVector AdjustedLocation = SurfaceHit.ImpactPoint + SurfaceHit.Normal * SurfaceOffset;

			UDecalComponent* Splatter = UGameplayStatics::SpawnDecalAtLocation(
				GetWorld(),
				BloodDecalMaterial,
				FVector(decalSize * 1.5f),
				AdjustedLocation,
				SurfaceRotation,
				20.0f
			);

			if (Splatter)
			{
				Splatter->SetFadeScreenSize(0.001f);
			}
		}
	}
}






void AGun::BulletHoleDecal(const FHitResult& Hit)
{
	FRotator DecalRotation = Hit.Normal.Rotation();

	// Spawn the decal at the hit location with the correct rotation and scale
	UGameplayStatics::SpawnDecalAtLocation(
		GetWorld(),
		BulletDecalMaterial,  // The material 
		FVector(bdecalSize, bdecalSize, bdecalSize), 
		Hit.Location, 
		DecalRotation,  
		60.0f  
	);
	
}
void AGun::ApplyRecoilTranslation()
{
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OwnerCharacter);
	if (PlayerCharacter && PlayerCharacter->ArmsRoot)
	{
		// Get the player's current view direction (including pitch)
		FVector CameraLoc;
		FRotator CameraRot;
		APlayerController* PC = Cast<APlayerController>(PlayerCharacter->GetController());

		if (!PC)
		{
			return;
		}

		PC->GetPlayerViewPoint(CameraLoc, CameraRot);
		FVector RecoilDirection = -CameraRot.Vector();  // Negative = push back relative to view
		RecoilDirection.Normalize();

		if (!bRecoilApplied)
		{
			bRecoilApplied = true;
			OriginalArmsRootLocation = PlayerCharacter->ArmsRoot->GetRelativeLocation();

			// Apply recoil instantly in direction opposite to camera view
			FVector RecoilTranslation = RecoilDirection * RecoilAmount;
			PlayerCharacter->ArmsRoot->AddWorldOffset(RecoilTranslation);

			// Set up interpolation recovery
			RecoilStartLocation = PlayerCharacter->ArmsRoot->GetRelativeLocation();
			RecoilTargetLocation = OriginalArmsRootLocation;
			RecoilRecoveryElapsed = 0.0f;
			bIsRecoveringFromRecoil = true;
		}
	}
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




