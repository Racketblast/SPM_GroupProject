// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalV2.h"

#include "PlayerCharacter.h"
#include "Components/ArrowComponent.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#define FPS 40

// Sets default values
APortalV2::APortalV2()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
		
	PortalSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("PortalSceneComponent"));
	PortalMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMeshComponent"));
	PortalMeshComponent->SetupAttachment(PortalSceneComponent);
	PortalPositionComponent = CreateDefaultSubobject<USceneComponent>(TEXT("PortalPositionComponent"));
	PortalPositionComponent->SetupAttachment(PortalSceneComponent);
	PortalDistanceTriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("PortalDistanceTriggerVolume"));
	PortalDistanceTriggerVolume->SetupAttachment(PortalSceneComponent);
	PortalTriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("PortalTriggerVolume"));
	PortalTriggerVolume->SetupAttachment(PortalSceneComponent);
	ForwardDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("ForwardDirection"));
	ForwardDirection->SetupAttachment(PortalSceneComponent);
	PortalSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("PortalSoundComponent"));
	PortalSoundComponent->SetupAttachment(PortalSceneComponent);

	TargetDeltaTime = 1.0f / FPS;
	AccumulatedTime = 0.0f;
}

// Called when the game starts or when spawned
void APortalV2::BeginPlay()
{
	Super::BeginPlay();

	SetTickGroup(TG_PostUpdateWork);

	PlayerCamera = UGameplayStatics::GetPlayerCameraManager(GetWorld(),0);
	
	SetPortalMaterial();

	if (PortalDistanceTriggerVolume)
	{
		PortalDistanceTriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &APortalV2::OnTriggerCloseBeginOverlap);
		PortalDistanceTriggerVolume->OnComponentEndOverlap.AddDynamic(this, &APortalV2::OnTriggerCloseEndOverlap);
	}
	
	FVector NewVector = ForwardDirection->GetForwardVector() * TextureOffsetAmount;
	FLinearColor NewPosition = UKismetMathLibrary::MakeColor(NewVector.X, NewVector.Y, NewVector.Z);
	PortalMaterialInstance->SetVectorParameterValue(FName("TextureOffset"), NewPosition);
	
	PortalMaterialInstance->SetVectorParameterValue(FName("PortalColor"), PortalColor);
}

void APortalV2::OnTriggerCloseBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA<APlayerCharacter>())
	{
		bBlendingToFaraway = false;
		bPlayerInRange = true;
	}
}

void APortalV2::OnTriggerCloseEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor->IsA<APlayerCharacter>())
	{
		bBlendingToFaraway = true;
		bPlayerInRange = false;
	}
}

void APortalV2::SetPortalMaterial()
{
	if (PortalMaterial)
	{
		PortalMaterialInstance = UMaterialInstanceDynamic::Create(PortalMaterial, this);
		PortalMeshComponent->SetMaterial(0, PortalMaterialInstance);
	}
}

void APortalV2::ChangePortalMaterial(float DeltaTime)
{
	if (bBlendingToFaraway && DistanceBlendAlpha != 1)
	{
		DistanceBlendAlpha = FMath::Clamp(DistanceBlendAlpha + DeltaTime * BlendSpeed, 0.0f, 1.0f);
		if (PortalMaterialInstance)
		{
			PortalMaterialInstance->SetScalarParameterValue(TEXT("BlendAmount"), DistanceBlendAlpha);
		}
	}
	else if (!bBlendingToFaraway && DistanceBlendAlpha != 0)
	{
		DistanceBlendAlpha = FMath::Clamp(DistanceBlendAlpha - DeltaTime * BlendSpeed, 0.0f, 1.0f);
		if (PortalMaterialInstance)
		{
			PortalMaterialInstance->SetScalarParameterValue(TEXT("BlendAmount"), DistanceBlendAlpha);
		}
	}
}

FVector APortalV2::MirrorAndTransformDirection(const FTransform& SourceTransform, const FTransform& TargetTransform, const FVector& Direction)
{
	FVector InverseDirection = UKismetMathLibrary::InverseTransformDirection(SourceTransform,Direction);
	InverseDirection = UKismetMathLibrary::MirrorVectorByNormal(InverseDirection,{1,0,0});
	InverseDirection = UKismetMathLibrary::MirrorVectorByNormal(InverseDirection,{0,1,0});
	return UKismetMathLibrary::TransformDirection(TargetTransform, InverseDirection);
}


void APortalV2::UpdatePortalPosition()
{
	if (!PlayerCamera || !LinkedPortal || !LinkedPortal->PortalPositionComponent)
	return;
	
	FVector ScaleVector = GetActorScale();
	ScaleVector.X *= -1;
	ScaleVector.Y *= -1;
	
	FTransform ReverseTransform = UKismetMathLibrary::MakeTransform(GetActorLocation(), GetActorRotation(), ScaleVector);
	FVector InverseLocation = UKismetMathLibrary::InverseTransformLocation(ReverseTransform, PlayerCamera->GetCameraLocation());
	FVector LinkedLocation = UKismetMathLibrary::TransformLocation(LinkedPortal->GetActorTransform(),InverseLocation);
	
	FRotator PlayerCameraRotation = PlayerCamera->GetCameraRotation();
	FVector CameraForwardDirection = MirrorAndTransformDirection(GetActorTransform(), LinkedPortal->GetActorTransform(), FRotationMatrix(PlayerCameraRotation).GetUnitAxis(EAxis::X));
	FVector CameraRightDirection = MirrorAndTransformDirection(GetActorTransform(), LinkedPortal->GetActorTransform(), FRotationMatrix(PlayerCameraRotation).GetUnitAxis(EAxis::Y));
	FVector CameraUpDirection = MirrorAndTransformDirection(GetActorTransform(), LinkedPortal->GetActorTransform(), FRotationMatrix(PlayerCameraRotation).GetUnitAxis(EAxis::Z));
		
	FRotator LinkedRotation = UKismetMathLibrary::MakeRotationFromAxes(CameraForwardDirection, CameraRightDirection, CameraUpDirection);
	
	LinkedPortal->PortalPositionComponent->SetWorldLocationAndRotation(LinkedLocation, LinkedRotation);
}

void APortalV2::ShouldTeleport()
{
	TArray<AActor*> Overlaps;
	PortalTriggerVolume->GetOverlappingActors(Overlaps);
	if (Overlaps.Num() > 0)
	{
		for (AActor* OverlapActor : Overlaps)
		{
			//Every Actor can teleport, not just player
			if (IsPointCrossingPortal(OverlapActor->GetActorLocation(), GetActorLocation(), ForwardDirection->GetForwardVector()))
			{
				if (PortalSoundComponent)
				{
					PortalSoundComponent->Play();
				}
				Teleport(OverlapActor);
			}
		}
	}
}

bool APortalV2::IsPointCrossingPortal(FVector Point, FVector PortalLocation, FVector PortalNormal)
{
	float CrossingDot = UKismetMathLibrary::Dot_VectorVector(PortalNormal, Point-PortalLocation);
	bool bIsInFront = false;
	if (CrossingDot >= 0)
	{
		bIsInFront = true;
	}
	FPlane PortalPlane = UKismetMathLibrary::MakePlaneFromPointAndNormal(PortalLocation,PortalNormal);

	float T;
	FVector Intersection;
	bool bIsIntersect = UKismetMathLibrary::LinePlaneIntersection(LastPosition,Point,PortalPlane,T,Intersection);

	bool bIsCrossing = false;
	if (bIsIntersect && !bIsInFront && bLastInFront)
	{
		bIsCrossing = true;
	}

	bLastInFront = bIsInFront;
	LastPosition = Point;
	return bIsCrossing;
}

void APortalV2::Teleport(AActor* OtherActor)
{
	if (!OtherActor || !LinkedPortal)
		return;
	//Makes it so the portal doesn't teleport itself
	if (OtherActor == this || OtherActor == LinkedPortal)
		return;

	FVector ScaleVector = GetActorScale();
	ScaleVector.X *= -1;
	ScaleVector.Y *= -1;
	FTransform ReverseTransform = UKismetMathLibrary::MakeTransform(GetActorLocation(), GetActorRotation(), ScaleVector);
	FVector InverseLocation = UKismetMathLibrary::InverseTransformLocation(ReverseTransform, OtherActor->GetActorLocation());
	FVector NewLocation = UKismetMathLibrary::TransformLocation(LinkedPortal->GetActorTransform(), InverseLocation);

	FRotator OtherActorRotation = OtherActor->GetActorRotation();
	FVector OtherActorForwardDirection = MirrorAndTransformDirection(GetActorTransform(), LinkedPortal->GetActorTransform(), FRotationMatrix(OtherActorRotation).GetUnitAxis(EAxis::X));
	FVector OtherActorRightDirection = MirrorAndTransformDirection(GetActorTransform(), LinkedPortal->GetActorTransform(), FRotationMatrix(OtherActorRotation).GetUnitAxis(EAxis::Y));
	FVector OtherActorUpDirection = MirrorAndTransformDirection(GetActorTransform(), LinkedPortal->GetActorTransform(), FRotationMatrix(OtherActorRotation).GetUnitAxis(EAxis::Z));
	
	FRotator NewRotation = UKismetMathLibrary::MakeRotationFromAxes(OtherActorForwardDirection, OtherActorRightDirection, OtherActorUpDirection);
	
	FTransform NewTransform = UKismetMathLibrary::MakeTransform(NewLocation, NewRotation, OtherActor->GetActorScale());
	OtherActor->SetActorTransform(NewTransform);

	if (OtherActor->IsA<APlayerCharacter>())
	{
		APlayerController* Controller = UGameplayStatics::GetPlayerController(this,0);

		FRotator ControllerRotation = Controller->GetControlRotation();
		FVector ControllerForwardDirection = MirrorAndTransformDirection(GetActorTransform(), LinkedPortal->GetActorTransform(), FRotationMatrix(ControllerRotation).GetUnitAxis(EAxis::X));
		FVector ControllerRightDirection = MirrorAndTransformDirection(GetActorTransform(), LinkedPortal->GetActorTransform(), FRotationMatrix(ControllerRotation).GetUnitAxis(EAxis::Y));
		FVector ControllerUpDirection = MirrorAndTransformDirection(GetActorTransform(), LinkedPortal->GetActorTransform(), FRotationMatrix(ControllerRotation).GetUnitAxis(EAxis::Z));
	
		NewRotation = UKismetMathLibrary::MakeRotationFromAxes(ControllerForwardDirection, ControllerRightDirection, ControllerUpDirection);
		Controller->SetControlRotation(NewRotation);
	}
	//Retains velocity even when flipped
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
	{
		FVector CurrentVelocity = Player->GetCharacterMovement()->Velocity;
		FVector NewVelocity = UpdateVelocity(CurrentVelocity);
		Player->GetCharacterMovement()->Velocity = NewVelocity;
	}
	else
	{
		if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(OtherActor->GetRootComponent()))
		{
			FVector CurrentVelocity = PrimComp->GetPhysicsLinearVelocity();
			FVector NewVelocity = UpdateVelocity(CurrentVelocity);
			PrimComp->SetPhysicsLinearVelocity(NewVelocity);
		}
	}
}

FVector APortalV2::UpdateVelocity(FVector Velocity)
{
	const float OriginalSpeed = Velocity.Size();

	Velocity.Normalize();

	FVector InverseVector = UKismetMathLibrary::InverseTransformDirection(GetActorTransform(), Velocity);
	InverseVector = UKismetMathLibrary::MirrorVectorByNormal(InverseVector, {1, 0, 0});
	InverseVector = UKismetMathLibrary::MirrorVectorByNormal(InverseVector, {0, 1, 0});
	FVector TransformedDirection = UKismetMathLibrary::TransformDirection(LinkedPortal->GetActorTransform(), InverseVector);

	FVector ReturnVector = OriginalSpeed * TransformedDirection;
	return ReturnVector;
}


// Called every frame
void APortalV2::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	ChangePortalMaterial(DeltaTime);
	
	if (!bPlayerInRange || !PlayerCamera || !LinkedPortal)
		return;
	AccumulatedTime += DeltaTime;

	if (AccumulatedTime >= TargetDeltaTime)
	{
		AccumulatedTime -= TargetDeltaTime;
		UpdatePortalPosition();
		ShouldTeleport();
	}
}