// Fill out your copyright notice in the Description page of Project Settings.

#include "TurretBuildingPart.h"
#include "TurretProjectile.h"
#include "AIChar.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================================
// linear algebra in gaming (turret targeting and rotation):
// we calculate the euclidean distance between two 3d points using the distance
// formula sqrt((x2-x1)^2 + (y2-y1)^2 + (z2-z1)^2). to aim at the target, we
// subtract the turret location vector from the target location vector and
// normalize it into a unit direction vector, then convert that direction vector
// into a yaw rotation angle.
// ============================================================================

ATurretBuildingPart::ATurretBuildingPart()
{
	// enable tick for target searching and rotation tracking
	PrimaryActorTick.bCanEverTick = true;

	// create muzzle component attachment point for projectile spawning
	MuzzleComponent = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleComponent"));
	MuzzleComponent->SetupAttachment(PivotArrow);
	MuzzleComponent->SetRelativeLocation(FVector(100.0f, 0.0f, 50.0f));
}

void ATurretBuildingPart::BeginPlay()
{
	Super::BeginPlay();

	// if spawned directly as active, start firing timer
	if (bIsPlaced)
	{
		OnPlaced();
	}
}

void ATurretBuildingPart::OnPlaced()
{
	bIsPlaced = true;

	// enable collision once placed
	if (Mesh)
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	}

	// start continuous firing timer
	GetWorld()->GetTimerManager().SetTimer(
		FireTimerHandle,
		this,
		&ATurretBuildingPart::FireProjectile,
		FireInterval,
		true
	);
}

void ATurretBuildingPart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// only track targets when placed
	if (!bIsPlaced)
	{
		return;
	}

	FindTarget();

	if (CurrentTarget && IsValid(CurrentTarget))
	{
		RotateTowardsTarget(DeltaTime);
	}
}

// finds nearest alive enemy within range
void ATurretBuildingPart::FindTarget()
{
	// check if current target is still valid and in range
	if (CurrentTarget && IsValid(CurrentTarget))
	{
		float Distance = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
		if (Distance <= TargetRange)
		{
			return;
		}
	}

	CurrentTarget = nullptr;
	float NearestDistance = TargetRange;

	TArray<AActor*> FoundEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAIChar::StaticClass(), FoundEnemies);

	for (AActor* EnemyActor : FoundEnemies)
	{
		AAIChar* AIEnemy = Cast<AAIChar>(EnemyActor);
		if (AIEnemy && IsValid(AIEnemy))
		{
			float Distance = FVector::Dist(GetActorLocation(), AIEnemy->GetActorLocation());
			if (Distance < NearestDistance)
			{
				NearestDistance = Distance;
				CurrentTarget = AIEnemy;
			}
		}
	}
}

// rotates turret towards target using interp
void ATurretBuildingPart::RotateTowardsTarget(float DeltaTime)
{
	if (!CurrentTarget) return;

	FVector TargetLocation = CurrentTarget->GetActorLocation();
	FVector TurretLocation = GetActorLocation();

	// calculate look at rotation focused on yaw
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(TurretLocation, TargetLocation);
	FRotator TargetRotation = FRotator(0.0f, LookAtRotation.Yaw, 0.0f);

	// smooth interpolation between current rotation and target rotation
	FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, RotationSpeed);
	SetActorRotation(NewRotation);
}

// spawns and fires projectile at target
void ATurretBuildingPart::FireProjectile()
{
	if (!bIsPlaced || !CurrentTarget || !IsValid(CurrentTarget) || !ProjectileClass)
	{
		return;
	}

	// verify target is within range
	float Distance = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
	if (Distance > TargetRange)
	{
		CurrentTarget = nullptr;
		return;
	}

	FVector SpawnLocation = MuzzleComponent ? MuzzleComponent->GetComponentLocation() : GetActorLocation();
	FVector Direction = (CurrentTarget->GetActorLocation() - SpawnLocation).GetSafeNormal();
	FRotator SpawnRotation = Direction.Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();

	GetWorld()->SpawnActor<ATurretProjectile>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);
}
