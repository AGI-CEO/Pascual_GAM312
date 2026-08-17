// Fill out your copyright notice in the Description page of Project Settings.

#include "TurretProjectile.h"
#include "AIChar.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"

// ============================================================================
// trace and collision in gaming (projectile physics & hits):
// game engines use simplified collision shapes like spheres and capsules to
// test overlaps and blocking hits each frame. when the sphere component intersects
// another bounding volume, the physics engine registers a hit event and returns
// contact normal and actor data so damage can be applied.
// ============================================================================

ATurretProjectile::ATurretProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// set up collision sphere as root component
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(15.0f);
	CollisionComp->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionComp->OnComponentHit.AddDynamic(this, &ATurretProjectile::OnHit);
	CollisionComp->SetCanEverAffectNavigation(false);
	RootComponent = CollisionComp;

	// set up static mesh for projectile bullet visual
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(CollisionComp);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// set up projectile movement component for linear trajectory
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 2000.0f;
	ProjectileMovement->MaxSpeed = 2000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	// destroy projectile after 3 seconds if no target is hit
	InitialLifeSpan = 3.0f;
}

// handles collision impact and applies damage
void ATurretProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// ignore collision with self or owner turret
	if (OtherActor && OtherActor != this && OtherActor != GetOwner())
	{
		// apply damage using unreal engine damage pipeline
		UGameplayStatics::ApplyDamage(
			OtherActor,
			Damage,
			GetInstigatorController(),
			this,
			UDamageType::StaticClass()
		);

		// explicitly apply damage to ai character
		AAIChar* Enemy = Cast<AAIChar>(OtherActor);
		if (Enemy)
		{
			FDamageEvent DamageEvent;
			Enemy->TakeDamage(Damage, DamageEvent, GetInstigatorController(), this);
		}

		// destroy bullet on contact
		Destroy();
	}
}
