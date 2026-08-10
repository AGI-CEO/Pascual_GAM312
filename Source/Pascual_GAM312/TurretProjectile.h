// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "TurretProjectile.generated.h"

// TurretProjectile is spawned by ATurretBuildingPart. It flies forward at a set speed
// and deals damage to enemy characters (AAIChar) upon impact before destroying itself.
UCLASS()
class PASCUAL_GAM312_API ATurretProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	ATurretProjectile();

	// Collision sphere component
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Projectile")
	USphereComponent* CollisionComp;

	// Visual mesh for the projectile
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Projectile")
	UStaticMeshComponent* ProjectileMesh;

	// Movement component handling trajectory and velocity
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Movement")
	UProjectileMovementComponent* ProjectileMovement;

	// Damage dealt to enemies hit by this projectile
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float Damage = 25.0f;

	// Collision hit event handler
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
