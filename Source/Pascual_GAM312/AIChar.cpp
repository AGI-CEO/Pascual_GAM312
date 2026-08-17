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
// a* (a-star) to search polygon graph nodes, finding optimal paths from the ai
// position to random wander points while navigating around landscape and objects.
// ============================================================================

AAIChar::AAIChar()
{
	// disable tick since wander behavior is driven by ai controller blueprint
	PrimaryActorTick.bCanEverTick = false;

	// enable overlap events on the capsule component to detect player contact
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AAIChar::BeginPlay()
{
	Super::BeginPlay();

	// bind capsule overlap event to deal contact damage to player
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AAIChar::OnOverlapBegin);
}

// handles incoming damage from turret projectiles or weapons
float AAIChar::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	Health -= ActualDamage;

	// destroy character when health drops to zero
	if (Health <= 0.0f)
	{
		Destroy();
	}

	return ActualDamage;
}

// deals damage to player on touch
void AAIChar::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bCanAttack && OtherActor)
	{
		APlayerChar* Player = Cast<APlayerChar>(OtherActor);
		if (Player && !Player->bIsDead)
		{
			bCanAttack = false;

			// apply damage to player using unreal engine damage system
			UGameplayStatics::ApplyDamage(
				Player,
				AttackDamage,
				GetController(),
				this,
				UDamageType::StaticClass()
			);

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("AI attacked player!"));
			}

			// start cooldown timer before ai can attack player again
			GetWorld()->GetTimerManager().SetTimer(
				AttackCooldownTimerHandle,
				this,
				&AAIChar::ResetAttack,
				AttackCooldown,
				false
			);
		}
	}
}

// resets attack cooldown so ai can attack again
void AAIChar::ResetAttack()
{
	bCanAttack = true;
}
