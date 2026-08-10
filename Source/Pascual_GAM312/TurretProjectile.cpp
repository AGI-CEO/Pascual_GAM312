// Fill out your copyright notice in the Description page of Project Settings.

#include "TurretProjectile.h"
#include "AIChar.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"

ATurretProjectile::ATurretProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// Set up collision sphere as root component
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(15.0f);
	CollisionComp->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionComp->OnComponentHit.AddDynamic(this, &ATurretProjectile::OnHit);
	CollisionComp->SetCanEverAffectNavigation(false);
	RootComponent = CollisionComp;

	// Set up static mesh component for visual bullet
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(CollisionComp);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Set up projectile movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 2000.0f;
	ProjectileMovement->MaxSpeed = 2000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f; // Linear trajectory

	// Destroy projectile after 3 seconds if it misses all targets
	InitialLifeSpan = 3.0f;
}

void ATurretProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Don't hit self or owner turret
	if (OtherActor && OtherActor != this && OtherActor != GetOwner())
	{
		// Apply damage using Unreal Engine's standard damage system
		UGameplayStatics::ApplyDamage(
			OtherActor,
			Damage,
			GetInstigatorController(),
			this,
			UDamageType::StaticClass()
		);

		// Also check directly for AAIChar for explicit handling
		AAIChar* Enemy = Cast<AAIChar>(OtherActor);
		if (Enemy)
		{
			FDamageEvent DamageEvent;
			Enemy->TakeDamage(Damage, DamageEvent, GetInstigatorController(), this);
		}

		Destroy();
	}
}
