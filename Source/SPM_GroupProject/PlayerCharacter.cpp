// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
//#include "Projectile.h"
//#include "ProjectileSpawner.h"
#include "Engine/StaticMeshSocket.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(RootComponent);
	PlayerCamera->bUsePawnControlRotation = true;
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::Jump);
	PlayerInputComponent->BindAction("Shoot", IE_Pressed, this, &APlayerCharacter::Shoot);
	PlayerInputComponent->BindAxis("MoveForward",this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight",this, &APlayerCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Yaw",this, &APlayerCharacter::Yaw);
	PlayerInputComponent->BindAxis("Pitch",this, &APlayerCharacter::Pitch);
	
}

void APlayerCharacter::MoveForward(float Value)
{
	FVector ForwardDirection = GetActorForwardVector();
	AddMovementInput(ForwardDirection, Value);

}
void APlayerCharacter::MoveRight(float Value)
{
	FVector RightDirection = GetActorRightVector();
	AddMovementInput(RightDirection, Value);

}


void APlayerCharacter::Yaw(float Value)
{
	AddControllerYawInput(Value);
}
void APlayerCharacter::Pitch(float Value)
{
	AddControllerPitchInput(Value);
}
void APlayerCharacter::Shoot()
{
	//GetWorld()->SpawnActor<AProjectile>(ProjectileActor, GetActorLocation(), GetActorRotation());
	
}