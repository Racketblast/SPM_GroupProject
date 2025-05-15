// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridManager.generated.h"


struct FGridNode
{
	FVector WorldLocation;
	FIntVector GridIndex;
	bool bIsWalkable = true;

	float GCost = 0;
	float HCost = 0;
	float FCost() const { return GCost + HCost; }

	TSharedPtr<FGridNode> Parent;

	FGridNode()	: WorldLocation(FVector::ZeroVector),
	  GridIndex(FIntVector::ZeroValue),
	  bIsWalkable(true),
	  GCost(0.f),
	  HCost(0.f),
	  Parent(nullptr) {}
	FGridNode(FVector InLocation, FIntVector InIndex)
		: WorldLocation(InLocation), GridIndex(InIndex), bIsWalkable(true) {}
};

UCLASS()
class SPM_GROUPPROJECT_API AGridManager : public AActor
{
	GENERATED_BODY()
	
public:
	AGridManager();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GridSizeX = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GridSizeY = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GridSizeZ = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CellSize = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableDebug = false;

	TMap<FIntVector, TSharedPtr<FGridNode>> Grid;

	TSharedPtr<FGridNode> GetNodeAt(const FIntVector& Index) const;

	void GenerateGrid();
	FVector GetWorldLocationFromIndex(const FIntVector& Index) const;
	FIntVector GetIndexFromWorldLocation(const FVector& Location) const;

	void UpdateNodeWalkability(const FIntVector& Index);
	
	TArray<FVector> FindPath(const FVector& StartLocation, const FVector& EndLocation); 

protected:
	virtual void BeginPlay() override;

};
