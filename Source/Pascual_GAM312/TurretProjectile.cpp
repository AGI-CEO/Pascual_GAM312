// Fill out your copyright notice in the Description page of Project Settings.

#include "TurretProjectile.h"
#include "AIChar.h"
#include "PlayerChar.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"

// ============================================================================
// trace and collision in gaming (projectile physics & hits):
// simplified collision shapes like spheres test overlaps and blocking hits.
// when collision occurs, the physics engine registers a hit event and returns
// contact normal and actor data so damage can be applied.
// ============================================================================

ATurretProjectile::ATurretProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(35.0f);
	CollisionComp->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionComp->SetGenerateOverlapEvents(true);
	CollisionComp->OnComponentHit.AddDynamic(this, &ATurretProjectile::OnHit);
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ATurretProjectile::OnOverlapBegin);
	CollisionComp->SetCanEverAffectNavigation(false);
	RootComponent = CollisionComp;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(CollisionComp);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 2500.0f;
	ProjectileMovement->MaxSpeed = 2500.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	Damage = 50.0f;
	InitialLifeSpan = 4.0f;
}

void ATurretProjectile::BeginPlay()
{
	Super::BeginPlay();

	// ignore collision with owner turret
	if (GetOwner())
	{
		CollisionComp->IgnoreActorWhenMoving(GetOwner(), true);
	}
}

void ATurretProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// proximity hit check ensures damage is dealt even if high-speed projectile passes through
	if (TargetActor && IsValid(TargetActor))
	{
		float Distance = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
		if (Distance <= 130.0f)
		{
			ApplyProjectileDamage(TargetActor);
		}
	}
}

void ATurretProjectile::ApplyProjectileDamage(AActor* OtherActor)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner() || OtherActor->IsA<APlayerChar>())
	{
		return;
	}

	// check if target is an AAIChar
	AAIChar* EnemyAI = Cast<AAIChar>(OtherActor);
	if (EnemyAI)
	{
		EnemyAI->Health -= Damage;

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FString::Printf(TEXT("Enemy AI took hit! Health: %.0f/100"), FMath::Max(0.0f, EnemyAI->Health)));
		}

		if (EnemyAI->Health <= 0.0f)
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Enemy AI eliminated!"));
			}
			EnemyAI->Destroy();
		}

		Destroy();
		return;
	}

	// if target is any other non-player character, destroy it
	if (OtherActor->IsA<ACharacter>())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Enemy eliminated!"));
		}
		OtherActor->Destroy();
		Destroy();
		return;
	}

	// if hitting terrain, walls, or obstacles, destroy projectile
	Destroy();
}

void ATurretProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ApplyProjectileDamage(OtherActor);
}

void ATurretProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ApplyProjectileDamage(OtherActor);
}
