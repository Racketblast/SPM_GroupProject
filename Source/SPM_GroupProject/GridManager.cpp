// Fill out your copyright notice in the Description page of Project Settings.


#include "GridManager.h"
#include "DrawDebugHelpers.h"
#include "Containers/Queue.h"
#include "Algo/Reverse.h"

AGridManager::AGridManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGridManager::BeginPlay()
{
	Super::BeginPlay();
	GenerateGrid();
}

void AGridManager::GenerateGrid()
{
	Grid.Empty();

	for (int32 x = 0; x < GridSizeX; ++x)
	{
		for (int32 y = 0; y < GridSizeY; ++y)
		{
			for (int32 z = 0; z < GridSizeZ; ++z)
			{
				FIntVector Index(x, y, z);
				FVector WorldPos = GetActorLocation() + FVector(x, y, z) * CellSize;

				TSharedPtr<FGridNode> NewNode = MakeShared<FGridNode>(WorldPos, Index);

				// Kollar efter hinder
				bool bHit = GetWorld()->OverlapBlockingTestByChannel(
					WorldPos,
					FQuat::Identity,
					ECC_WorldStatic,
					FCollisionShape::MakeSphere(CellSize * 0.4f)
				);

				NewNode->bIsWalkable = !bHit;

				// Debug
				FColor Color = NewNode->bIsWalkable ? FColor::Green : FColor::Red;
				DrawDebugBox(GetWorld(), WorldPos, FVector(CellSize * 0.5f), Color, true, -1.f, 0, 5.f);

				Grid.Add(Index, NewNode);
			}
		}
	}
}

TSharedPtr<FGridNode> AGridManager::GetNodeAt(const FIntVector& Index) const
{
	if (Grid.Contains(Index))
		return Grid[Index];
	return nullptr;
}

FVector AGridManager::GetWorldLocationFromIndex(const FIntVector& Index) const
{
	return GetActorLocation() + FVector(Index) * CellSize;
}

FIntVector AGridManager::GetIndexFromWorldLocation(const FVector& Location) const
{
	FVector Local = Location - GetActorLocation();
	return FIntVector(
		FMath::FloorToInt(Local.X / CellSize),
		FMath::FloorToInt(Local.Y / CellSize),
		FMath::FloorToInt(Local.Z / CellSize));
}

// används inte just nu, men kanske kommer att behövas, bra ifall man har rörliga hinder med
void AGridManager::UpdateNodeWalkability(const FIntVector& Index)
{
	if (auto Node = GetNodeAt(Index))
	{
		bool bHit = GetWorld()->OverlapBlockingTestByChannel(
			Node->WorldLocation,
			FQuat::Identity,
			ECC_WorldStatic,
			FCollisionShape::MakeSphere(CellSize * 0.4f)
		);

		Node->bIsWalkable = !bHit;

		//Debug
		FColor Color = Node->bIsWalkable ? FColor::Green : FColor::Red;
		DrawDebugBox(GetWorld(), Node->WorldLocation, FVector(CellSize * 0.5f), Color, true, -1.f, 0, 5.f);
	}
}

TArray<FVector> AGridManager::FindPath(const FVector& StartLocation, const FVector& EndLocation)
{


	// Ingen path hittades
	return {};
}




