// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AIChar.generated.h"

// ai character class linked to ai controller for pathfinding and wander behavior
UCLASS()
class PASCUAL_GAM312_API AAIChar : public ACharacter
{
	GENERATED_BODY()

public:
	// constructor setting defaults
	AAIChar();

	// current health of the enemy character
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Health = 100.0f;

	// damage dealt to player on contact
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackDamage = 15.0f;

	// cooldown between contact attacks in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackCooldown = 1.5f;

	// tracks whether ai can attack right now
	bool bCanAttack = true;

	// called when taking damage from turret projectiles
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// handles capsule overlap with player to deal damage
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// resets attack cooldown
	void ResetAttack();

protected:
	virtual void BeginPlay() override;

private:
	FTimerHandle AttackCooldownTimerHandle;
};
