// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "PlayerGameInstance.h"
#include "BuyBox.h"
#include "Teleporter.h"
#include "Projectile.h"
//#include "ProjectileSpawner.h"
#include "Engine/StaticMeshSocket.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
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
	APlayerController* PC = Cast<APlayerController>(GetController());
	UE_LOG(LogTemp, Warning, TEXT("Hello"));
	if (PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("Hello"));

		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}

}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const FVector Start = PlayerCamera->GetComponentLocation();
	const FVector End = Start + (PlayerCamera->GetForwardVector() * UseDistance);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false);
	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
	{
		TargetActor = HitResult.GetActor();
	}
	else
	{
		TargetActor = nullptr;
	}

}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::Jump);
	PlayerInputComponent->BindAction("Shoot", IE_Pressed, this, &APlayerCharacter::Shoot);
	PlayerInputComponent->BindAction("Use", IE_Pressed,this, &APlayerCharacter::Use);
	PlayerInputComponent->BindAction("Reload", IE_Pressed,this, &APlayerCharacter::Reload);
	PlayerInputComponent->BindAction("SelectWeapon1", IE_Pressed,this, &APlayerCharacter::SelectWeapon1);
	PlayerInputComponent->BindAction("SelectWeapon2", IE_Pressed,this, &APlayerCharacter::SelectWeapon2);
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
	if (Weapon1Equipped)
	{
		if (Ammo1 > 0)
		{
			GetWorld()->SpawnActor<AProjectile>(Weapon1, GetActorLocation(), GetActorRotation());
			Ammo1--;
		}
	}
	if (Weapon2Equipped)
	{
		if (Ammo2 > 0)
		{
			GetWorld()->SpawnActor<AProjectile>(Weapon2, GetActorLocation(), GetActorRotation());
			Ammo2--;
		}
	}
	
	
	
}
void APlayerCharacter::Reload()
{
	if (Weapon1Equipped)
    {
	Ammo1 = MaxAmmo1;
    }
	if (Weapon2Equipped)
	{
		Ammo2 = MaxAmmo2;
	}
	
	
	
}
void APlayerCharacter::SelectWeapon1()
{
	if (!Weapon1Equipped)
	{
		Weapon1Equipped = true;
		Weapon2Equipped = false;
		
		GetWorld()->SpawnActor<AProjectile>(GWeapon1, GetActorLocation(), GetActorRotation());
		//GWeapon1->SetupAttachment(RootComponent);
	}
	
	
}
void APlayerCharacter::SelectWeapon2()
{

	if (!Weapon2Equipped)
	{
		Weapon2Equipped = true;
		Weapon1Equipped = false;
		GetWorld()->SpawnActor<AProjectile>(GWeapon2, GetActorLocation(), GetActorRotation());
		
	}
		
}

void APlayerCharacter::Use()
{
	if (TargetActor)
	{
		UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
		//Teleporting function
		if (ATeleporter* Teleporter = Cast<ATeleporter>(TargetActor))
		{
			if (GI)
			{
				if (Teleporter->TargetLevelName != "Hub")
				{
					GI->Level += 1;
				}
				else
				{
					GI->Money += 20;
				}
			
				UE_LOG(LogTemp, Warning, TEXT("Level: %d"), GI->Level);
				UE_LOG(LogTemp, Warning, TEXT("Money: %d"), GI->Money);
				UGameplayStatics::OpenLevel(this, Teleporter->TargetLevelName);
			}
		}
		//Buying function
		if (const ABuyBox* BuyBox = Cast<ABuyBox>(TargetActor))
		{
			if (GI)
			{
				if (!GI->HasBought(BuyBox->TargetUpgradeName))
				{
					if (BuyBox->TargetUpgradeCost <= GI->Money)
					{
						GI->Money -= BuyBox->TargetUpgradeCost;
						GI->UpgradeArray.Add(BuyBox->TargetUpgradeName);
						GI->CurrentWeapon = BuyBox->TargetUpgradeName;
						
					}
				}
				else
				{
					GI->CurrentWeapon = BuyBox->TargetUpgradeName;
				}
			}
		}
	}
}