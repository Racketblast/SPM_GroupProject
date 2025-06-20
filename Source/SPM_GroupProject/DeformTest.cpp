#include "DeformTest.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

ADeformTest::ADeformTest()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
}

void ADeformTest::BeginPlay()
{
	Super::BeginPlay();

	if (MeshComp && MeshComp->GetMaterial(0))
	{
		DynMaterial = MeshComp->CreateAndSetMaterialInstanceDynamic(0);
	}
}

void ADeformTest::ApplyDeformation(const FVector& WorldHitLocation)
{
	if (!DynMaterial || !MeshComp) return;

	// Convert world hit location to local mesh space
	FVector LocalHitLocation = MeshComp->GetComponentTransform().InverseTransformPosition(WorldHitLocation);

	DynMaterial->SetVectorParameterValue(FName("HitLocation"), FLinearColor(LocalHitLocation));
	DynMaterial->SetScalarParameterValue(FName("Strength"), 50.f);
	DynMaterial->SetScalarParameterValue(FName("Radius"), 100.f);
}
