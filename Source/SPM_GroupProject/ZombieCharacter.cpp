#include "ZombieCharacter.h"
#include "FracturedLimbActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"

AZombieCharacter::AZombieCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AZombieCharacter::BeginPlay()
{
	Super::BeginPlay();
	InitializeLimbHealth();
}

void AZombieCharacter::InitializeLimbHealth()
{
	if (!GetMesh()) return;

	// Get all bone names from the skeletal mesh
	const int32 BoneCount = GetMesh()->GetNumBones();
	for (int32 i = 0; i < BoneCount; ++i)
	{
		FName BoneName = GetMesh()->GetBoneName(i);

		// Only add bones that are simulated (optional, can filter further if needed)
		if (!LimbHealth.Contains(BoneName))
		{
			LimbHealth.Add(BoneName, DefaultLimbHealth);
		}
	}
}


void AZombieCharacter::HandleBoneHit(FName BoneName, FVector HitLocation, float DamageAmount)
{
	if (BoneName == NAME_None || DestroyedBones.Contains(BoneName))
		return;

	// Reduce health for this bone
	if (LimbHealth.Contains(BoneName))
	{
		LimbHealth[BoneName] -= DamageAmount;

		if (LimbHealth[BoneName] <= 0.f)
		{
			DestroyedBones.Add(BoneName);

			// Hide bone
			GetMesh()->HideBoneByName(BoneName, EPhysBodyOp::PBO_None);

			// Spawn fractured limb
			if (FracturedLimbClass)
			{
				FVector SpawnLocation = HitLocation + FVector(0, 0, 10.0f);
				FRotator SpawnRotation = FRotator::ZeroRotator;

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				AFracturedLimbActor* Fractured = GetWorld()->SpawnActor<AFracturedLimbActor>(
					FracturedLimbClass,
					SpawnLocation,
					SpawnRotation,
					SpawnParams
				);

				if (Fractured)
				{
					// Set the fractured limb's scale to match the zombie mesh scale (or tweak as needed)
					FVector CharacterMeshScale = GetMesh()->GetComponentScale();
					Fractured->SetActorScale3D(CharacterMeshScale);

					UE_LOG(LogTemp, Log, TEXT("Fractured bone spawned: %s with scale %s"), *BoneName.ToString(), *CharacterMeshScale.ToString());
				}
			}
		}
	}
}
