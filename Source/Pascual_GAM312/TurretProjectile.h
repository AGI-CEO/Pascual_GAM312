// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "TurretProjectile.generated.h"

// projectile spawned by turret building part that flies toward targets and deals damage
UCLASS()
class PASCUAL_GAM312_API ATurretProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	ATurretProjectile();

	virtual void Tick(float DeltaTime) override;

	// collision sphere component
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Projectile")
	USphereComponent* CollisionComp;

	// visual mesh for the projectile
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Projectile")
	UStaticMeshComponent* ProjectileMesh;

	// movement component handling trajectory and velocity
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Movement")
	UProjectileMovementComponent* ProjectileMovement;

	// damage dealt to enemies hit by this projectile
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float Damage = 50.0f;

	// target actor this projectile is homing/flying towards
	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
	AActor* TargetActor = nullptr;

	// collision hit event handler
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// collision overlap event handler
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void ApplyProjectileDamage(AActor* OtherActor);

protected:
	virtual void BeginPlay() override;
};
