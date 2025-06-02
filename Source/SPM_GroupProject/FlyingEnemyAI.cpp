// Fill out your copyright notice in the Description page of Project Settings.

#include "FlyingEnemyAI.h"
#include "FlyingAI_Controller.h"
#include "EnemyAIUtils.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"


// Sätter AI controller klassen som används till FlyingAI_Controller.
// Sätter också en setting till PlacedInWorldOrSpawned, så att den automatiskt får sin AI controller när den placeras i världen eller spawnas.
AFlyingEnemyAI::AFlyingEnemyAI()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AFlyingAI_Controller::StaticClass();
}

// Sätter fienden till flying mode för unreals CharacterMovementcomponent, sätter även PlayerInRange variablen för AFlyingAI_Controller. 
void AFlyingEnemyAI::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	
	if (AFlyingAI_Controller* AIController = Cast<AFlyingAI_Controller>(GetController()))
	{
		AIController->SetPlayerInRange(PlayerInRange);
		//UE_LOG(LogTemp, Warning, TEXT("PlayerInRange %f"), PlayerInRange);
	}
}

void AFlyingEnemyAI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//DrawFlyableZRange();

	// Kollar om fienden inte har rört sig på ett tag, och däö försöker den röra sig lie åt sidorna istället för bara upp och ner,
	// detta är ifall fienden är fast i taket eller golvet. 
	if (!CurrentTargetLocation.IsNearlyZero())
	{
		FVector CurrentLocation = GetActorLocation();
		
		if (CurrentTargetLocation.Z < CurrentLocation.Z - 10.0f) 
		{
			StuckCheckTimer += DeltaTime;

			if (StuckCheckTimer >= 1.0f) 
			{
				float ZDelta = FMath::Abs(CurrentLocation.Z - LastLocation.Z);

				if (ZDelta < 5.0f)
				{
					bIsDescendingStuck = true;
					TryLateralUnstick(); 
				}
				else
				{
					bIsDescendingStuck = false;
				}

				LastLocation = CurrentLocation;
				StuckCheckTimer = 0.0f;
			}
		}
	}

	// För teleporteringen av fienden, efter att en timer har runnit ut och att spelaren inte ser fienden. 
	if (bIsMovingToTarget)
	{
		float TimeSinceStarted = GetWorld()->GetTimeSeconds() - DestinationStartTime;
		if (TimeSinceStarted >= MaxTimeToReachDestination)
		{
			AFlyingAI_Controller* AIController = Cast<AFlyingAI_Controller>(GetController());
			ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

			bool bHasLineOfSight = AIController && AIController->HasLineOfSightToPlayer();
			bool bInRange = false;
			bool bPlayerSeesEnemy = IsVisibleToPlayer();

			UE_LOG(LogTemp, Warning, TEXT("Timer PlayerSeesEnemy: %s"), bPlayerSeesEnemy ? TEXT("true") : TEXT("false"));

			if (Player)
			{
				float DistanceToPlayer = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
				bInRange = DistanceToPlayer <= PlayerInRange;

				if (DistanceToPlayer <= TeleportProximityThreshold)
				{
					UE_LOG(LogTemp, Warning, TEXT("Skipping teleport: already near player (%.2f)"), DistanceToPlayer);
					bIsMovingToTarget = false;
					return;
				}
			}

			// Teleporterar bara om fienden inte redan är inrange och har line of sight
			// och ifall teleportering när spelaren är på eller ifall spelaren inte ser fienden
			if ((!(bHasLineOfSight && bInRange)) && (bCanTeleportIfVisibleToPlayer || !bPlayerSeesEnemy))
			{
				TeleportToValidLocationNearPlayer();
			}

			bIsMovingToTarget = false;
		}
	}

	// Fixar rotationen för fienden så att den alltid tittar mot spelaren. 
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

	if (!Player) return;

	FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
	FRotator LookAtRotation = FRotationMatrix::MakeFromX(ToPlayer).Rotator();

	SetActorRotation(LookAtRotation);
}

// Teleporterar fienden, använder sig av FindValidTeleportLocation funktionen från EnemyAIUtils klassen som jag gjort
void AFlyingEnemyAI::TeleportToValidLocationNearPlayer()
{
	if (!Controller) return;

	FVector TargetLocation = GetCurrentTargetLocation();
	FVector NewTeleportLocation;
	
	if (UEnemyAIUtils::FindValidTeleportLocation(this, TargetLocation, NewTeleportLocation, bAvoidFrontTeleport) && bTeleportAfterTimer)
	{
		SetActorLocation(NewTeleportLocation);
		NotifyTeleported();
		UE_LOG(LogTemp, Warning, TEXT("Enemy teleported after timer ran out."));
	}
}

// Används för teleport för fienden. Den kållar om spelaren ser fienden och om spelaren gör det så ska fienden inte få teleportera sig.
bool AFlyingEnemyAI::IsVisibleToPlayer() const
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PC || !Player) return false;

	FVector PlayerViewLoc;
	FRotator PlayerViewRot;
	PC->GetPlayerViewPoint(PlayerViewLoc, PlayerViewRot);

	// Kållar om fienden är framför spelaren
	FVector DirectionToEnemy = (GetActorLocation() - PlayerViewLoc).GetSafeNormal();
	float Dot = FVector::DotProduct(PlayerViewRot.Vector(), DirectionToEnemy);
	
	//UE_LOG(LogTemp, Warning, TEXT("Dot to enemy: %.2f"), Dot);
	
	if (Dot < 0.5f) // Runt 60 degrees, så ungefär 120 grader syn fält
	{
		return false;
	}

	// Är fienden synlig för spelaren, alltså line of sight, fienden är alltså inte bakom en vägg exempelvis. 
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(Player);

	// Ignorera också alla andra flygande fiender från line of sight checken. 
	for (TActorIterator<AFlyingEnemyAI> It(GetWorld()); It; ++It)
	{
		AFlyingEnemyAI* OtherEnemy = *It;
		if (OtherEnemy && OtherEnemy != this)
		{
			Params.AddIgnoredActor(OtherEnemy);
		}
	}

	// Ignorera alla objekt man kan see igenom (tittar efter en tag)
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor->Tags.Contains("SeeThroughObject"))
		{
			Params.AddIgnoredActor(Actor);
		}
	}

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, PlayerViewLoc, GetActorLocation(), ECC_Visibility, Params);
	
	/*if (bHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit actor: %s"), *GetNameSafe(Hit.GetActor()));  
	}*/

	return !bHit || Hit.GetActor() == this;
}

// Ifall fienden är fast, så försöker den åka lite åt sidorna. För att försöka undvika att den bara flyger in i väggar. Inte perfekt dock!
void AFlyingEnemyAI::TryLateralUnstick()
{
	FVector LateralDirection;

	// Väljer antingen vänster eller höger
	if (FMath::RandBool())
	{
		LateralDirection = GetActorRightVector();
	}
	else
	{
		LateralDirection = -GetActorRightVector();
	}

	FVector NewTarget = GetActorLocation() + LateralDirection * 300.0f; // rör sig till sidan

	// liten push uppåt
	NewTarget.Z += 100.0f;

	FVector Direction = (NewTarget - GetActorLocation()).GetSafeNormal();
	AddMovementInput(Direction, 1.0f, false);
}

void AFlyingEnemyAI::SetMaxAltitude(float Altitude)
{
	MaxAltitude = Altitude;
}

float AFlyingEnemyAI::GetMaxAltitude() const
{
	return MaxAltitude;
}

void AFlyingEnemyAI::SetMinAltitude(float Altitude)
{
	MinAltitude = Altitude;
}

float AFlyingEnemyAI::GetMinAltitude() const
{
	return MinAltitude;
}

void AFlyingEnemyAI::SetCurrentTargetLocation(const FVector& NewTarget)
{
	CurrentTargetLocation = NewTarget;
}

FVector AFlyingEnemyAI::GetCurrentTargetLocation() const
{
	return CurrentTargetLocation;
}

// Sätter igång en timer som kållar hur länge sedan det var som den teleporterade sig. 
void AFlyingEnemyAI::NotifyTeleported()
{
	LastTeleportTime = GetWorld()->GetTimeSeconds();
}

// Kollar om fienden får attackera, gör så att den inte kan det ifall den precis har teleporerat sig. 
bool AFlyingEnemyAI::CanShoot() const
{
	return (GetWorld()->GetTimeSeconds() - LastTeleportTime) >= PostTeleportFireDelay;
}

// Kållar om cooldownen för att fienden ska få attackera är över. 
bool AFlyingEnemyAI::IsFireCooldownElapsed() const
{
	return (GetWorld()->GetTimeSeconds() - LastFireTime) >= FireCooldown;
}

// Används för att se när fienden attackerade senast, så att man kan se till att den inte attackerar för ofta med en cooldown.
// Startar även om timern för teleporteringen, vilket den också gär i IsMoving(), som är precis nedan denna funktion. 
void AFlyingEnemyAI::NotifyFired()
{
	LastFireTime = GetWorld()->GetTimeSeconds();
	DestinationStartTime = GetWorld()->GetTimeSeconds(); 
}

// Anropas varje gång fienden börjar röra sig till en ny target location.
// Detta används för teleportens timer, så att fienden teleporteras ifall den tar för lång tid på sig att ta sig till spelaren. 
void AFlyingEnemyAI::IsMoving()
{
	//UE_LOG(LogTemp, Warning, TEXT("IsMoving."));
	if (!bIsMovingToTarget)
	{
		//UE_LOG(LogTemp, Warning, TEXT("bIsMovingToTarget is false."));
		DestinationStartTime = GetWorld()->GetTimeSeconds(); 
		bIsMovingToTarget = true;
	}
}

// För debuging
void AFlyingEnemyAI::DrawFlyableZRange() const
{
	FVector ActorLocation = GetActorLocation();

	// Debug sphere för min altitude 
	FVector MinZLocation = FVector(ActorLocation.X, ActorLocation.Y, MinAltitude);
	DrawDebugSphere(GetWorld(), MinZLocation, 40.f, 12, FColor::Red, false, -1.f, 0, 2.f);

	// Debug sphere för max altitude
	FVector MaxZLocation = FVector(ActorLocation.X, ActorLocation.Y, MaxAltitude);
	DrawDebugSphere(GetWorld(), MaxZLocation, 40.f, 12, FColor::Green, false, -1.f, 0, 2.f);

	// Debug linje som är mellan dem två debug sphere 
	DrawDebugLine(GetWorld(), MinZLocation, MaxZLocation, FColor::Yellow, false, -1.f, 0, 1.f);
}