// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_Main.h"

#include "MoneyBox.h"
#include "WaveManager.h"
#include "AI_Controller.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "GameFramework/CharacterMovementComponent.h"
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
				GetWorld()->SpawnActor<AMoneyBox>(MoneyBoxClass, GetTransform());
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
	
	// check if AI is jumping/falling across nav links
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		if (MovementComp->IsFalling() || MovementComp->IsFlying())
		{
			// Skippar teleport midair
			return;
		}
	}

	FVector CurrentLocation = GetActorLocation();
	float DistanceMovedSq = FVector::DistSquared(CurrentLocation, LastKnownLocation);

	// reset timer
	if (DistanceMovedSq > MinMoveDistance * MinMoveDistance)
	{
		LastKnownLocation = CurrentLocation;
		TimeSinceLastMovement = 3.f;
	}
	else
	{
		TimeSinceLastMovement += DeltaTime;
	}

	// If stuck or off-navmesh, set Blackboard flag
	if (TimeSinceLastMovement > StuckCheckInterval || IsOutsideNavMesh())
	{
		if (AAI_Controller* AICont = Cast<AAI_Controller>(GetController()))
		{
			if (UBlackboardComponent* BB = AICont->GetBlackboardComponent())
			{
				BB->SetValueAsBool(FName("TeleportToNavmesh"), true);
			}
			
		}
	}
}

// Called to bind functionality to input
void AAI_Main::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}


bool AAI_Main::IsOutsideNavMesh() const
{
	const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys) return false;

	FNavLocation NavLocation;
	return !NavSys->ProjectPointToNavigation(GetActorLocation(), NavLocation);
}

