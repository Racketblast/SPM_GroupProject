#include "HitscanGun.h"
#include "WaveManager.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
#include "MoneyBox.h"
#include "PlayerCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "UObject/UnrealType.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

void AHitscanGun::BeginPlay()
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
}

void AHitscanGun::Fire(FVector FireLocation, FRotator FireRotation)
{
    if (bIsReloading)
    {
        return;
    }

    const float CurrentTime = GetWorld()->GetTimeSeconds();
    const float TimeBetweenShots = 1.0f / RoundsPerSecond;

    if (CurrentTime - LastFireTime < TimeBetweenShots || CurrentAmmo <= 0)
    {
        return;
    }

    LastFireTime = CurrentTime;

    // 🔊 Play fire sound using audio component
    if (FireSound && FireAudioComponent)
    {
        if (FireAudioComponent->IsPlaying())
        {
            FireAudioComponent->Stop();
        }
        FireAudioComponent->SetSound(FireSound);
        FireAudioComponent->Play();
    }

    FVector ShotDirection = FireRotation.Vector();
    FVector End = FireLocation + (ShotDirection * Range);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    if (OwnerCharacter)
    {
        Params.AddIgnoredActor(OwnerCharacter);
    }

    if (GetWorld()->LineTraceSingleByChannel(Hit, FireLocation, End, ECC_Visibility, Params))
    {
        DrawDebugLine(GetWorld(), FireLocation, Hit.ImpactPoint, FColor::Red, false, 1.0f, 0, 1.0f);

        AActor* HitActor = Hit.GetActor();
        LastHitActor = HitActor;

        if (ACharacter* HitCharacter = Cast<ACharacter>(HitActor))
        {
            bEnemyHit = true;
            EnemyHitFalse();
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
        else if (HitActor)
        {
            if (HitActor->FindFunction("OnLineTraceHit"))
            {
                HitActor->ProcessEvent(HitActor->FindFunction("OnLineTraceHit"), nullptr);
            }
        }
    }
    else
    {
        DrawDebugLine(GetWorld(), FireLocation, End, FColor::Blue, false, 1.0f, 0, 1.0f);
    }

    CurrentAmmo--;

    if (OwnerCharacter)
    {
        APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
        if (PC)
        {
            float RecoilPitch = FMath::FRandRange(RecoilPitchMin, RecoilPitchMax);
            float RecoilYaw = FMath::FRandRange(RecoilYawMin, RecoilYawMax);

            FRotator ControlRotation = PC->GetControlRotation();
            ControlRotation.Pitch -= RecoilPitch;
            ControlRotation.Yaw += RecoilYaw;

            PC->SetControlRotation(ControlRotation);
        }
    }
    
}
void AHitscanGun::EnemyHitFalse()
{
    bEnemyHit = false;
}

