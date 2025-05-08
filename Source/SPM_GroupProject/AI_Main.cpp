// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_Main.h"

#include "MoneyBox.h"
#include "WaveManager.h"
#include "EngineUtils.h"

// Sets default values
AAI_Main::AAI_Main()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

UBehaviorTree* AAI_Main::GetBehaviorTree() const
{
	return BehaviorTree;
}

float AAI_Main::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	if (!bIsDead)
	{
		AIHealth -= DamageAmount;
		UE_LOG(LogTemp, Warning, TEXT("AI Takedamage"));
		if (AIHealth <= 0)
		{
		UE_LOG(LogTemp, Error, TEXT("AI dead"));
			AIHealth = 0;
			bIsDead = true;
			TSubclassOf<AMoneyBox> MoneyBoxClass = LoadClass<AMoneyBox>(nullptr, TEXT("/Game/Blueprints/BP_MoneyBox.BP_MoneyBox_C"));
			if (MoneyBoxClass)
			{
				FTransform SpawnTransform = GetTransform();
				FVector NewLocation = SpawnTransform.GetLocation();
				NewLocation.Z -= 100.0f;
				SpawnTransform.SetLocation(NewLocation);
				GetWorld()->SpawnActor<AMoneyBox>(MoneyBoxClass, SpawnTransform);
			}
			
			for (TActorIterator<AWaveManager> It(GetWorld()); It; ++It)
			{
				if (AWaveManager* WaveManager = *It)
				{
					WaveManager->OnEnemyKilled();
					break;
				}
			}
			
			Destroy();
		}
	}
	
	return DamageAmount;
}

// Called when the game starts or when spawned
void AAI_Main::BeginPlay()
{
	Super::BeginPlay();

	AIHealth = MaxAIHealth;
}

// Called every frame
void AAI_Main::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AAI_Main::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

