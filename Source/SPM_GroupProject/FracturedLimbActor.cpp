#include "FracturedLimbActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Kismet/KismetMathLibrary.h"

AFracturedLimbActor::AFracturedLimbActor()
{
    PrimaryActorTick.bCanEverTick = false;

    GeometryCollectionComp = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollectionComp"));
    RootComponent = GeometryCollectionComp;

    GeometryCollectionComp->SetSimulatePhysics(true);
    GeometryCollectionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GeometryCollectionComp->SetCollisionObjectType(ECC_PhysicsBody);
    GeometryCollectionComp->SetCollisionResponseToAllChannels(ECR_Block);
    GeometryCollectionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    GeometryCollectionComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
}

void AFracturedLimbActor::BeginPlay()
{
    Super::BeginPlay();

    if (FractureAsset && GeometryCollectionComp)
    {
        GeometryCollectionComp->SetRestCollection(FractureAsset);
        GeometryCollectionComp->RecreatePhysicsState();

        // Apply a strong impulse to trigger fracture immediately
        FVector Impulse = FVector(
            FMath::FRandRange(-1.f, 1.f),
            FMath::FRandRange(-1.f, 1.f),
            FMath::FRandRange(0.5f, 1.f)) * 500.f;

        GeometryCollectionComp->AddImpulse(Impulse, NAME_None, true);
    }
}
