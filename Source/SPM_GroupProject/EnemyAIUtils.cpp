// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIUtils.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

// Hittar en giltig position nära spelaren som fienden kan teleportera till. 
bool UEnemyAIUtils::FindValidTeleportLocation(APawn* Pawn, const FVector& TargetLocation, FVector& OutLocation, bool bAvoidFrontTeleport)
{
	AActor* Player = UGameplayStatics::GetPlayerPawn(Pawn->GetWorld(), 0);
	if (!Player) return false;

	FVector PlayerLocation = Player->GetActorLocation();
	FVector PlayerForward = Player->GetActorForwardVector();

	// Försöker hitta en plats runt spelaren där fienden kan teleporteras.
	// bAvoidFront styr om funktionen ska undvika att placera fienden framför spelaren.
	auto TryFindLocation = [&](bool bAvoidFront) -> bool
	{
		// Försöker upp till 20 slumpmässiga positioner runt spelaren.
		for (int32 i = 0; i < 20; ++i)
		{
			// Skapar ett slumpmässigt avstånd i en slumpmässig riktning (vinkel 0–360) runt spelaren i X/Y planet.
			float AngleDegrees = FMath::RandRange(0.f, 360.f);
			float Angle = FMath::DegreesToRadians(AngleDegrees);
			float Radius = FMath::RandRange(200.f, 600.f);
			FVector Offset = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * Radius;

			// Höjer positionen som fienden vill teleportera sig till
			FVector TestLocation = PlayerLocation + Offset;
			TestLocation.Z += 200.f;

			//  Är platsen framför spelaren, Dot blir > 0.5 om platsen är framför spelaren.
			FVector ToTestLocation = (TestLocation - PlayerLocation).GetSafeNormal();
			float Dot = FVector::DotProduct(PlayerForward, ToTestLocation);

			// Kollar om det finns fri sikt från platsen till spelaren och om platsen är tom och utan kollision.
			bool bLineClear = !Pawn->GetWorld()->LineTraceTestByChannel(TestLocation, PlayerLocation, ECC_Visibility);
			bool bIsFlyable = IsFlyableLocation(Pawn, Pawn->GetWorld(), TestLocation, 50.f);

			// Tillåter endast positioner som inte är framför spelaren om bAvoidFront är true
			// Om alla villkor är uppfyllda, sätt OutLocation och returnera true.
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

	// Om det första försöket misslyckas, så körs ett annat där fienden får teleportera till spelarens field of view så att dem ser teleporteringen
	// Dock om bAvoidFrontTeleport är satt till true, så kommer den aldrig att teleportera till en position framför spelaren
	return !bAvoidFrontTeleport && TryFindLocation(false);
}

// Kollar om det går att flyga på den angivna platsen utan att krocka med något
bool UEnemyAIUtils::IsFlyableLocation(APawn* Pawn, UWorld* World, const FVector& Location, float ClearanceRadius)
{
	// Ignorerar både fienden själv och spelaren från OverlapBlockingTestByChannel
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Pawn);
	Params.AddIgnoredActor(UGameplayStatics::GetPlayerPawn(World, 0));

	// Kållar om stället är tomt, så att inga hinder är ivägen, Skapar en osynlig sfär vid Location.
	return !World->OverlapBlockingTestByChannel(
		Location,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(ClearanceRadius),
		Params
	);
}



