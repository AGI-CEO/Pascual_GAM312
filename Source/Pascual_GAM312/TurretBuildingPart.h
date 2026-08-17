// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuildingPart.h"
#include "TurretBuildingPart.generated.h"

class ATurretProjectile;

// turret building part that can be crafted and placed. when active, it scans side-to-side,
// tracks enemy ai characters, and fires projectiles.
UCLASS()
class PASCUAL_GAM312_API ATurretBuildingPart : public ABuildingPart
{
	GENERATED_BODY()
	
public:
	ATurretBuildingPart();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// muzzle component attachment point for spawning projectiles
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Turret")
	USceneComponent* MuzzleComponent;

	// projectile class to spawn when firing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	TSubclassOf<ATurretProjectile> ProjectileClass;

	// time between shots in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	float FireInterval = 1.0f;

	// maximum detection range for enemies
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	float TargetRange = 3000.0f;

	// rotation tracking speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	float RotationSpeed = 8.0f;

	// whether turret is active and placed in the world
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	bool bIsPlaced = false;

	// called when turret is placed
	UFUNCTION(BlueprintCallable, Category = "Turret")
	void OnPlaced();

	// fires projectile at target
	UFUNCTION()
	void FireProjectile();

private:
	// finds nearest enemy character within range
	void FindTarget();

	// rotates turret to face current target
	void RotateTowardsTarget(float DeltaTime);

	// idle scanning motion side-to-side
	void IdleScan(float DeltaTime);

	// timer handle for firing
	FTimerHandle FireTimerHandle;

	// pointer to current target actor
	UPROPERTY()
	AActor* CurrentTarget = nullptr;

	// base yaw angle captured when placed
	float BasePlacementYaw = 0.0f;
};
