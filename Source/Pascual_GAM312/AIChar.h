// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AIChar.generated.h"

// ai character class with pathfinding, combat contact damage, and health
UCLASS()
class PASCUAL_GAM312_API AAIChar : public ACharacter
{
	GENERATED_BODY()

public:
	AAIChar();

	virtual void Tick(float DeltaTime) override;

	// health of the enemy character
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Health = 100.0f;

	// damage dealt to player on contact
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackDamage = 15.0f;

	// attack cooldown in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackCooldown = 1.5f;

	// attack range in units
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackRange = 160.0f;

	// whether ai can attack
	bool bCanAttack = true;

	// handles taking damage from turret projectiles
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// deals contact attack damage to player
	void AttackPlayer(class APlayerChar* Player);

	// resets attack cooldown
	void ResetAttack();

protected:
	virtual void BeginPlay() override;

private:
	FTimerHandle AttackCooldownTimerHandle;
};
