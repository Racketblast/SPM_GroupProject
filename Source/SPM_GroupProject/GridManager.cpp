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
				if (bEnableDebug)
				{
					FColor Color = NewNode->bIsWalkable ? FColor::Green : FColor::Red;
					DrawDebugBox(GetWorld(), WorldPos, FVector(CellSize * 0.5f), Color, true, -1.f, 0, 5.f);
				}

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
		if (bEnableDebug)
		{
			FColor Color = Node->bIsWalkable ? FColor::Green : FColor::Red;
			DrawDebugBox(GetWorld(), Node->WorldLocation, FVector(CellSize * 0.5f), Color, true, -1.f, 0, 5.f);
		}
	}
}

TArray<FVector> AGridManager::FindPath(const FVector& StartLocation, const FVector& EndLocation)
{
    FIntVector StartIndex = GetIndexFromWorldLocation(StartLocation);
    FIntVector EndIndex = GetIndexFromWorldLocation(EndLocation);

    auto StartNode = GetNodeAt(StartIndex);
    auto EndNode = GetNodeAt(EndIndex);

    if (!StartNode.IsValid() || !EndNode.IsValid() || !EndNode->bIsWalkable)
    {
        return {};
    }

    TArray<TSharedPtr<FGridNode>> OpenSet;
    TSet<FIntVector> ClosedSet;

    OpenSet.Add(StartNode);

    StartNode->GCost = 0;
    StartNode->HCost = FVector::Dist(StartNode->WorldLocation, EndNode->WorldLocation);
    StartNode->Parent = nullptr;

    while (OpenSet.Num() > 0)
    {
        // Get node with lowest FCost
        OpenSet.Sort([](const TSharedPtr<FGridNode>& A, const TSharedPtr<FGridNode>& B) {
            return A->FCost() < B->FCost();
        });

        TSharedPtr<FGridNode> CurrentNode = OpenSet[0];
        OpenSet.RemoveAt(0);
        ClosedSet.Add(CurrentNode->GridIndex);

        if (CurrentNode->GridIndex == EndIndex)
        {
            // Reconstruct path
            TArray<FVector> Path;
            while (CurrentNode.IsValid())
            {
                Path.Add(CurrentNode->WorldLocation);
                CurrentNode = CurrentNode->Parent;
            }
            Algo::Reverse(Path);
            return Path;
        }

        // Get neighbors
        for (int32 dx = -1; dx <= 1; ++dx)
        {
            for (int32 dy = -1; dy <= 1; ++dy)
            {
                for (int32 dz = -1; dz <= 1; ++dz)
                {
                    if (dx == 0 && dy == 0 && dz == 0)
                        continue;

                    FIntVector NeighborIndex = CurrentNode->GridIndex + FIntVector(dx, dy, dz);
                    auto Neighbor = GetNodeAt(NeighborIndex);

                    if (!Neighbor.IsValid() || !Neighbor->bIsWalkable || ClosedSet.Contains(NeighborIndex))
                        continue;

                    float NewGCost = CurrentNode->GCost + FVector::Dist(CurrentNode->WorldLocation, Neighbor->WorldLocation);

                    if (NewGCost < Neighbor->GCost || !OpenSet.Contains(Neighbor))
                    {
                        Neighbor->GCost = NewGCost;
                        Neighbor->HCost = FVector::Dist(Neighbor->WorldLocation, EndNode->WorldLocation);
                        Neighbor->Parent = CurrentNode;

                        if (!OpenSet.Contains(Neighbor))
                        {
                            OpenSet.Add(Neighbor);
                        }
                    }
                }
            }
        }
    }

    // Ingen väg hittades 
    return {};
}




