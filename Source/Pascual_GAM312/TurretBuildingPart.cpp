// Fill out your copyright notice in the Description page of Project Settings.

#include "TurretBuildingPart.h"
#include "TurretProjectile.h"
#include "AIChar.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"

ATurretBuildingPart::ATurretBuildingPart()
{
	// Enable tick for target searching and rotation tracking
	PrimaryActorTick.bCanEverTick = true;

	// Create MuzzleComponent attachment point for projectile spawning
	MuzzleComponent = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleComponent"));
	MuzzleComponent->SetupAttachment(PivotArrow);
	MuzzleComponent->SetRelativeLocation(FVector(100.0f, 0.0f, 50.0f));
}

void ATurretBuildingPart::BeginPlay()
{
	Super::BeginPlay();

	// If spawned directly as active (bIsPlaced true in editor), start firing timer
	if (bIsPlaced)
	{
		OnPlaced();
	}
}

void ATurretBuildingPart::OnPlaced()
{
	bIsPlaced = true;

	// Ensure collision is enabled once placed
	if (Mesh)
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	}

	// Start firing timer
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

	// Only search and track targets if the turret has been placed
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

void ATurretBuildingPart::FindTarget()
{
	// Check if current target is still valid and within range
	if (CurrentTarget && IsValid(CurrentTarget))
	{
		float Distance = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
		if (Distance <= TargetRange)
		{
			return; // Keep existing target
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

void ATurretBuildingPart::RotateTowardsTarget(float DeltaTime)
{
	if (!CurrentTarget) return;

	FVector TargetLocation = CurrentTarget->GetActorLocation();
	FVector TurretLocation = GetActorLocation();

	// Focus rotation on Yaw (horizontal aim)
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(TurretLocation, TargetLocation);
	FRotator TargetRotation = FRotator(0.0f, LookAtRotation.Yaw, 0.0f);

	FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, RotationSpeed);
	SetActorRotation(NewRotation);
}

void ATurretBuildingPart::FireProjectile()
{
	if (!bIsPlaced || !CurrentTarget || !IsValid(CurrentTarget) || !ProjectileClass)
	{
		return;
	}

	// Verify target is still within range
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
