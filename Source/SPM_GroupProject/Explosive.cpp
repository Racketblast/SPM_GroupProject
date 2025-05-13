#include "Explosive.h"
#include "CollisionQueryParams.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "UObject/UnrealType.h"
#include "MoneyBox.h"
#include "PlayerCharacter.h"
#include "WaveManager.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundBase.h"

AExplosive::AExplosive()
{
	PrimaryActorTick.bCanEverTick = false;

	ExplosiveMesh = CreateDefaultSubobject<UStaticMeshComponent>("ExplosiveMesh");
	RootComponent = ExplosiveMesh;

	ProjectileComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComponent"));
	ProjectileComponent->InitialSpeed = 1200.f;
	ProjectileComponent->bShouldBounce = true;
}

void AExplosive::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(ExplosionTimer, this, &AExplosive::Explode, ExplosionDelay, false);
}

void AExplosive::Explode()
{
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this);

	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FVector ExplosionCenter = GetActorLocation();

	GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		ExplosionCenter,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(ExplosionRadius),
		Params
	);

	if (ExplosionEffectAsset)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffectAsset, GetActorLocation(), FRotator::ZeroRotator, true);
	}

	if (ExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
	}

	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* HitActor = Result.GetActor();

		if (ACharacter* HitCharacter = Cast<ACharacter>(HitActor))
		{
			if (APlayerCharacter* Player = Cast<APlayerCharacter>(GetInstigator()))
			{
				Player->bEnemyHit = true;
				UE_LOG(LogTemp, Error, TEXT("hit activated (from explosive)"));
				Player->EnemyHitFalse();
			}


			static const FName AIHealthName = TEXT("AIHealth");

			if (FIntProperty* HealthProp = FindFProperty<FIntProperty>(HitCharacter->GetClass(), AIHealthName))
			{
				int32 CurrentHealth = HealthProp->GetPropertyValue_InContainer(HitCharacter);
				CurrentHealth -= static_cast<int32>(WeaponDamage);

				if (CurrentHealth <= 0)
				{
					TSubclassOf<AMoneyBox> MoneyBoxClass = LoadClass<AMoneyBox>(nullptr, TEXT("/Game/Blueprints/BP_MoneyBox.BP_MoneyBox_C"));
					if (MoneyBoxClass)
					{
						FTransform SpawnTransform = HitCharacter->GetTransform();
						FVector NewLocation = SpawnTransform.GetLocation();
						NewLocation.Z -= 100.0f;
						SpawnTransform.SetLocation(NewLocation);
						GetWorld()->SpawnActor<AMoneyBox>(MoneyBoxClass, SpawnTransform);
					}

					HitCharacter->Destroy();

					for (TActorIterator<AWaveManager> It(GetWorld()); It; ++It)
					{
						if (AWaveManager* WaveManager = *It)
						{
							WaveManager->OnEnemyKilled();
							break;
						}
					}
				}
				else
				{
					HealthProp->SetPropertyValue_InContainer(HitCharacter, CurrentHealth);
				}
			}
		}
		else if (HitActor && HitActor->FindFunction("OnLineTraceHit"))
		{
			HitActor->ProcessEvent(HitActor->FindFunction("OnLineTraceHit"), nullptr);
		}
	}

	Destroy();
}
