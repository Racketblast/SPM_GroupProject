// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIUtils.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

bool UEnemyAIUtils::FindValidTeleportLocation(APawn* Pawn, const FVector& TargetLocation, FVector& OutLocation)
{
	AActor* Player = UGameplayStatics::GetPlayerPawn(Pawn->GetWorld(), 0);
	if (!Player) return false;

	FVector PlayerLocation = Player->GetActorLocation();
	FVector PlayerForward = Player->GetActorForwardVector();

	auto TryFindLocation = [&](bool bAvoidFront) -> bool
	{
		for (int32 i = 0; i < 20; ++i)
		{
			float AngleDegrees = FMath::RandRange(0.f, 360.f);
			float Angle = FMath::DegreesToRadians(AngleDegrees);
			float Radius = FMath::RandRange(200.f, 600.f);
			FVector Offset = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * Radius;

			FVector TestLocation = PlayerLocation + Offset;
			TestLocation.Z += 200.f;

			FVector ToTestLocation = (TestLocation - PlayerLocation).GetSafeNormal();
			float Dot = FVector::DotProduct(PlayerForward, ToTestLocation);

			bool bLineClear = !Pawn->GetWorld()->LineTraceTestByChannel(TestLocation, PlayerLocation, ECC_Visibility);
			bool bIsFlyable = IsFlyableLocation(Pawn, Pawn->GetWorld(), TestLocation, 50.f);

			if ((Dot < 0.5f || !bAvoidFront) && bLineClear && bIsFlyable)
			{
				OutLocation = TestLocation;
				DrawDebugSphere(Pawn->GetWorld(), TestLocation, 50.f, 12, bAvoidFront ? FColor::Cyan : FColor::Green, false, 2.0f);
				return true;
			}
		}
		return false;
	};

	// Första försöket, undvik att teleportera framför spelaren, så att den ser teleporteringen
	if (TryFindLocation(true))
	{
		return true;
	}

	// Om det första försöket misslyckas, så körs ett annat där fienden får teleportera til spelarens field of view så att dem ser teleporteringen
	return TryFindLocation(false);
}

bool UEnemyAIUtils::IsFlyableLocation(APawn* Pawn, UWorld* World, const FVector& Location, float ClearanceRadius)
{
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Pawn);
	Params.AddIgnoredActor(UGameplayStatics::GetPlayerPawn(World, 0));

	// Kållar om stället är tomt, så att inga hinder är ivägen
	return !World->OverlapBlockingTestByChannel(
		Location,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(ClearanceRadius),
		Params
	);
}



