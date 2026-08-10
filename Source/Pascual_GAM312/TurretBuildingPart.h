// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuildingPart.h"
#include "TurretBuildingPart.generated.h"

class ATurretProjectile;
class AAIChar;

// TurretBuildingPart inherits from ABuildingPart so it can be crafted and placed
// using the existing building placement system. When placed, it scans for enemy AI
// characters (AAIChar) within range, rotates towards them, and fires projectiles.
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

	// Spawn location for projectiles (e.g. end of the turret barrel)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Turret")
	USceneComponent* MuzzleComponent;

	// The projectile Blueprint class to spawn when firing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	TSubclassOf<ATurretProjectile> ProjectileClass;

	// Time in seconds between shots (default 1.0s)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	float FireInterval = 1.0f;

	// Maximum distance to detect and engage enemy targets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	float TargetRange = 1000.0f;

	// Rotation speed when tracking targets (degrees per second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	float RotationSpeed = 5.0f;

	// Whether the turret is placed in the world (active) or currently in placement preview mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	bool bIsPlaced = false;

	// Called when player confirms placement of the turret
	UFUNCTION(BlueprintCallable, Category = "Turret")
	void OnPlaced();

	// Function called on a timer to fire projectiles at the current target
	UFUNCTION()
	void FireProjectile();

private:
	// Find nearest AAIChar target within TargetRange
	void FindTarget();

	// Rotate turret towards current target
	void RotateTowardsTarget(float DeltaTime);

	// Timer handle for continuous firing
	FTimerHandle FireTimerHandle;

	// Pointer to current enemy target
	UPROPERTY()
	AAIChar* CurrentTarget = nullptr;
};
