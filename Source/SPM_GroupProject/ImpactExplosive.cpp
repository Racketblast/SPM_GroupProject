#include "ImpactExplosive.h"
#include "Components/StaticMeshComponent.h"

AImpactExplosive::AImpactExplosive()
{
	// Enable simulation and collision
	if (ExplosiveMesh)
	{
		ExplosiveMesh->SetSimulatePhysics(true);
		ExplosiveMesh->SetNotifyRigidBodyCollision(true); // Enable hit events
	}
}

void AImpactExplosive::BeginPlay()
{
	Super::BeginPlay();

	if (ExplosiveMesh)
	{
		ExplosiveMesh->OnComponentHit.AddDynamic(this, &AImpactExplosive::OnHit);
	}
}

void AImpactExplosive::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Prevent multiple explosions

		Explode();
	
}

