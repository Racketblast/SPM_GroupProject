#include "PlayerCharacter.h"
#include "PlayerGameInstance.h"
#include "BuyBox.h"
#include "Teleporter.h"
#include "Projectile.h"
//#include "ProjectileSpawner.h"
#include "Gun.h"
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
	GetMesh()->SetupAttachment(PlayerCamera);
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
	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
	{
		if (GI->CurrentWeapon == WeaponName1)
		{
			APlayerCharacter::SelectWeapon1();
		}
		else if (GI->CurrentWeapon == WeaponName2)
		{
			APlayerCharacter::SelectWeapon2();
		}
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
	if (CurrentAmmo > 0)
	{
		
		if (Weapon1Equipped)
		{
			const FTransform SocketTransform = GetMesh()->GetSocketTransform(TEXT("hand_lSocket"));
			FRotator SocketRot = SocketTransform.GetRotation().Rotator();
			FRotator SpawnRotation( SocketRot.Pitch+12, GetActorRotation().Yaw,  SocketRot.Roll);
			GetWorld()->SpawnActor<AProjectile>(Projectile1, SocketTransform.GetLocation(), SpawnRotation);
		}
		if (Weapon2Equipped)
		{
			const FTransform SocketTransform = GetMesh()->GetSocketTransform(TEXT("hand_lSocket"));
			FRotator SocketRot = SocketTransform.GetRotation().Rotator();
			FRotator SpawnRotation( SocketRot.Pitch+12, GetActorRotation().Yaw,  SocketRot.Roll);
			GetWorld()->SpawnActor<AProjectile>(Projectile2, SocketTransform.GetLocation(), SpawnRotation);
		}
		CurrentAmmo--;
	}
}

void APlayerCharacter::Reload()
{
	if (Weapon2Equipped)
	{
		if (ExtraMags <= 0){
			return;
		}else
		{
			ExtraMags--;
		}
		
	}
	CurrentAmmo = CurrentMaxAmmo;
}

void APlayerCharacter::SelectWeapon1()
{
	if (UPlayerGameInstance* PlayerGameInstance = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		if (!Weapon1Equipped && PlayerGameInstance->HasBought(WeaponName1))
		{
			if (Weapon2Equipped)
			{
				CurrentGun->Destroy();
				UE_LOG(LogTemp, Warning, TEXT("Gun Destroyed"));
			}
			PlayerGameInstance->CurrentWeapon = WeaponName1;
     
			if (CurrentMaxAmmo > 0)
			{
				Ammo2 = CurrentAmmo;
			}
     
			Weapon1Equipped = true;
			Weapon2Equipped = false;

			CurrentAmmo = Ammo1;
			CurrentMaxAmmo = MaxAmmo1;
     
			if (!GetMesh()->DoesSocketExist(TEXT("hand_lSocket")))
			{
				UE_LOG(LogTemp, Warning, TEXT("hand_lSocket not found on mesh."));
				return;
			}

			// Get the socket transform from the mesh
			const FTransform SocketTransform = GetMesh()->GetSocketTransform(TEXT("hand_lSocket"));
			AGun* SpawnedGun = GetWorld()->SpawnActor<AGun>(GWeapon1);
			if (SpawnedGun)
			{
				SpawnedGun->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("hand_lSocket"));
				CurrentGun = SpawnedGun;
			}
		}
	}
}

void APlayerCharacter::SelectWeapon2()
{
	if (UPlayerGameInstance* PlayerGameInstance = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		if (!Weapon2Equipped && PlayerGameInstance->HasBought(WeaponName2))
		{if (Weapon1Equipped)
		{
			CurrentGun->Destroy();
			UE_LOG(LogTemp, Warning, TEXT("Gun Destroyed"));
		}
			PlayerGameInstance->CurrentWeapon = WeaponName2;
     
			if (CurrentMaxAmmo > 0)
			{
				Ammo1 = CurrentAmmo;
			}
     
			Weapon2Equipped = true;
			Weapon1Equipped = false;

			CurrentAmmo = Ammo2;
			CurrentMaxAmmo = MaxAmmo2;
     
			if (!GetMesh()->DoesSocketExist(TEXT("hand_lSocket")))
			{
				UE_LOG(LogTemp, Warning, TEXT("hand_lSocket not found on mesh."));
				return;
			}

			// Get the socket transform from the mesh
			const FTransform SocketTransform = GetMesh()->GetSocketTransform(TEXT("hand_lSocket"));
			AGun* SpawnedGun = GetWorld()->SpawnActor<AGun>(GWeapon2);
			if (SpawnedGun)
			{
				SpawnedGun->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("hand_lSocket"));
				CurrentGun = SpawnedGun;
			}
		}
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
						if (GI->CurrentWeapon == WeaponName1)
						{
							APlayerCharacter::SelectWeapon1();
						}
						else if (GI->CurrentWeapon == WeaponName2)
						{
							APlayerCharacter::SelectWeapon2();
						}
                 
					}
				}
				else
				{
					GI->CurrentWeapon = BuyBox->TargetUpgradeName;
					if (GI->CurrentWeapon == WeaponName1)
					{
						APlayerCharacter::SelectWeapon1();
					}
					else if (GI->CurrentWeapon == WeaponName2)
					{
						APlayerCharacter::SelectWeapon2();
					}
				}
			}
		}
	}
}
