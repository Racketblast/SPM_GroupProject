// Fill out your copyright notice in the Description page of Project Settings.


#include "projectile.h"

#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AProjectile::AProjectile()
{
	
	PrimaryActorTick.bCanEverTick = false;
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>("ProjectileMesh");
	RootComponent = ProjectileMesh;
	ProjectileComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComponent"));
	ProjectileComponent-> InitialSpeed = 9000.f; 
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(TCount);
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (TCount <= 0 )
	{
		Destroy();
	}

}

