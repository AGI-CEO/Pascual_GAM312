// Fill out your copyright notice in the Description page of Project Settings.

#include "AIChar.h"
#include "PlayerChar.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

// ============================================================================
// ai navigation mesh and pathfinding in game development:
// a navigation mesh (navmesh) rasterizes level geometry into flat 2d polygons
// representing walkable areas. the ai controller uses pathfinding algorithms like
// a* to navigate polygon nodes to reach wander destinations across terrain.
// ============================================================================

AAIChar::AAIChar()
{
	// enable tick to check distance to player for melee contact attacks
	PrimaryActorTick.bCanEverTick = true;

	Health = 100.0f;
	AttackDamage = 15.0f;
	AttackCooldown = 1.5f;
	AttackRange = 160.0f;
	bCanAttack = true;

	GetCapsuleComponent()->SetGenerateOverlapEvents(true);
}

void AAIChar::BeginPlay()
{
	Super::BeginPlay();
}

void AAIChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bCanAttack || Health <= 0.0f)
	{
		return;
	}

	// find player character in the level
	APlayerChar* Player = Cast<APlayerChar>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (Player && Player->Health > 0.0f)
	{
		float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
		if (Distance <= AttackRange)
		{
			AttackPlayer(Player);
		}
	}
}

// deals damage to player when in close proximity
void AAIChar::AttackPlayer(APlayerChar* Player)
{
	if (!Player || !bCanAttack || Health <= 0.0f) return;

	bCanAttack = false;

	// apply damage directly to player
	Player->SetHealth(-AttackDamage);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, FString::Printf(TEXT("AI attacked you! -%.0f HP (Health: %.0f/100)"), AttackDamage, Player->Health));
	}

	// start cooldown timer before next attack
	GetWorld()->GetTimerManager().SetTimer(
		AttackCooldownTimerHandle,
		this,
		&AAIChar::ResetAttack,
		AttackCooldown,
		false
	);
}

// handles incoming damage from turret projectiles
float AAIChar::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Health -= DamageAmount;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FString::Printf(TEXT("AI took damage! Health: %.0f/100"), FMath::Max(0.0f, Health)));
	}

	// destroy character when health drops to zero
	if (Health <= 0.0f)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Enemy AI eliminated!"));
		}
		Destroy();
	}

	return DamageAmount;
}

void AAIChar::ResetAttack()
{
	bCanAttack = true;
}
