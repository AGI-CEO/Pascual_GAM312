// Fill out your copyright notice in the Description page of Project Settings.

#include "TurretBuildingPart.h"
#include "TurretProjectile.h"
#include "AIChar.h"
#include "PlayerChar.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

// ============================================================================
// linear algebra in gaming (turret targeting and rotation):
// we calculate euclidean distance using vector subtraction and length. to aim,
// we subtract the turret location from the target location and normalize it into
// a unit direction vector, converting that vector into a yaw rotation angle.
// ============================================================================

ATurretBuildingPart::ATurretBuildingPart()
{
	PrimaryActorTick.bCanEverTick = true;

	TargetRange = 3000.0f;
	FireInterval = 1.0f;
	RotationSpeed = 8.0f;
	ProjectileClass = ATurretProjectile::StaticClass();

	MuzzleComponent = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleComponent"));
	MuzzleComponent->SetupAttachment(PivotArrow);
	MuzzleComponent->SetRelativeLocation(FVector(100.0f, 0.0f, 50.0f));
}

void ATurretBuildingPart::BeginPlay()
{
	Super::BeginPlay();

	BasePlacementYaw = GetActorRotation().Yaw;

	if (!ProjectileClass)
	{
		ProjectileClass = ATurretProjectile::StaticClass();
	}

	if (bIsPlaced)
	{
		OnPlaced();
	}
}

void ATurretBuildingPart::OnPlaced()
{
	bIsPlaced = true;
	BasePlacementYaw = GetActorRotation().Yaw;

	if (Mesh)
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	}

	if (!ProjectileClass)
	{
		ProjectileClass = ATurretProjectile::StaticClass();
	}

	// start continuous firing timer
	GetWorld()->GetTimerManager().SetTimer(
		FireTimerHandle,
		this,
		&ATurretBuildingPart::FireProjectile,
		FireInterval,
		true
	);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Turret online: scanning area!"));
	}
}

void ATurretBuildingPart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsPlaced)
	{
		return;
	}

	FindTarget();

	if (CurrentTarget && IsValid(CurrentTarget))
	{
		RotateTowardsTarget(DeltaTime);
	}
	else
	{
		IdleScan(DeltaTime);
	}
}

// idle scanning sweep side-to-side (oscillates yaw +-45 degrees)
void ATurretBuildingPart::IdleScan(float DeltaTime)
{
	float Time = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	float OffsetYaw = FMath::Sin(Time * 2.0f) * 45.0f;
	FRotator TargetRotation = FRotator(0.0f, BasePlacementYaw + OffsetYaw, 0.0f);
	FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 3.0f);
	SetActorRotation(NewRotation);
}

// finds nearest enemy character within range
void ATurretBuildingPart::FindTarget()
{
	// verify existing target is still valid and within range
	if (CurrentTarget && IsValid(CurrentTarget))
	{
		AAIChar* AICharTarget = Cast<AAIChar>(CurrentTarget);
		if (!AICharTarget || AICharTarget->Health > 0.0f)
		{
			float Distance = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
			if (Distance <= TargetRange)
			{
				return;
			}
		}
	}

	CurrentTarget = nullptr;
	float NearestDistance = TargetRange;

	// search for all characters in the level
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		// do not target the player or self
		if (Actor == this || Actor->IsA<APlayerChar>())
		{
			continue;
		}

		// if it's an AI character, ensure it is alive
		AAIChar* AIEnemy = Cast<AAIChar>(Actor);
		if (AIEnemy && AIEnemy->Health <= 0.0f)
		{
			continue;
		}

		float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
		if (Distance < NearestDistance)
		{
			NearestDistance = Distance;
			CurrentTarget = Actor;
		}
	}
}

// rotates turret towards current target
void ATurretBuildingPart::RotateTowardsTarget(float DeltaTime)
{
	if (!CurrentTarget || !IsValid(CurrentTarget)) return;

	FVector TargetLocation = CurrentTarget->GetActorLocation();
	FVector TurretLocation = GetActorLocation();

	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(TurretLocation, TargetLocation);
	FRotator TargetRotation = FRotator(0.0f, LookAtRotation.Yaw, 0.0f);

	FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, RotationSpeed);
	SetActorRotation(NewRotation);
}

// fires projectile at target
void ATurretBuildingPart::FireProjectile()
{
	if (!bIsPlaced)
	{
		return;
	}

	FindTarget();

	if (!CurrentTarget || !IsValid(CurrentTarget))
	{
		return;
	}

	AAIChar* AIEnemy = Cast<AAIChar>(CurrentTarget);
	if (AIEnemy && AIEnemy->Health <= 0.0f)
	{
		CurrentTarget = nullptr;
		return;
	}

	float Distance = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
	if (Distance > TargetRange)
	{
		CurrentTarget = nullptr;
		return;
	}

	if (!ProjectileClass)
	{
		ProjectileClass = ATurretProjectile::StaticClass();
	}

	FVector SpawnLocation = MuzzleComponent ? MuzzleComponent->GetComponentLocation() : (GetActorLocation() + GetActorForwardVector() * 100.0f + FVector(0.0f, 0.0f, 50.0f));
	FVector Direction = (CurrentTarget->GetActorLocation() - SpawnLocation).GetSafeNormal();
	FRotator SpawnRotation = Direction.Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATurretProjectile* SpawnedBullet = GetWorld()->SpawnActor<ATurretProjectile>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (SpawnedBullet)
	{
		SpawnedBullet->TargetActor = CurrentTarget;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, TEXT("Turret firing at enemy!"));
	}
}
