// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Resource_M.h"
#include "BuildingPart.h"
#include "PlayerWidget.h"
#include "ObjectiveWidget.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerChar.generated.h"

// player character class that controls movement, stats, inventory, crafting, and interaction
UCLASS()
class PASCUAL_GAM312_API APlayerChar : public ACharacter
{
	GENERATED_BODY()

public:
	// constructor setting up components and default values
	APlayerChar();

protected:
	// called when the game starts
	virtual void BeginPlay() override;

public:	
	// called every frame
	virtual void Tick(float DeltaTime) override;

	// binds input keys to gameplay functions
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// moves player forward and backward
	UFUNCTION()
	void MoveForward(float axisValue);

	// moves player left and right
	UFUNCTION()
	void MoveRight(float axisValue);

	// starts jumping
	UFUNCTION()
	void StartJump();

	// stops jumping
	UFUNCTION()
	void StopJump();

	// performs line trace to interact with resources or place buildings
	UFUNCTION()
	void FindObject();

	// --- player stat functions ---

	// changes health by amount and clamps between 0 and 100
	UFUNCTION(BlueprintCallable, Category = "Player Stats")
	void SetHealth(float amount);

	// changes hunger by amount and clamps between 0 and 100
	UFUNCTION(BlueprintCallable, Category = "Player Stats")
	void SetHunger(float amount);

	// changes stamina by amount and clamps between 0 and 100
	UFUNCTION(BlueprintCallable, Category = "Player Stats")
	void SetStamina(float amount);

	// timer function that drains hunger and regenerates stamina over time
	UFUNCTION(BlueprintCallable, Category = "Player Stats")
	void DecreaseStats();

	// eats a berry from inventory to restore hunger and stamina
	UFUNCTION(BlueprintCallable, Category = "Player Stats")
	void EatBerry();

	// handles taking damage from starvation or external attacks
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// called when player dies
	void Die();

	// blueprint event triggered on death to show the lose widget
	UFUNCTION(BlueprintImplementableEvent, Category = "Player Stats")
	void OnPlayerDeath();

	// adds collected resources to inventory array
	UFUNCTION(BlueprintCallable, Category = "Resources")
	void GiveResource(int32 amount, FString resourceType);

	// --- building system functions ---

	// spends wood and stone to craft building parts
	UFUNCTION(BlueprintCallable, Category = "Building")
	void UpdateResources(int32 woodAmount, int32 stoneAmount, FString buildingObject);

	// spawns a building part ahead of the player for placement
	UFUNCTION(BlueprintCallable, Category = "Building")
	void SpawnBuilding(int32 buildingID, bool& isSuccess);

	// rotates held building part by 90 degrees
	UFUNCTION()
	void RotateBuilding();

	// --- player stat variables ---

	// player health points (0 to 100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Health = 100.0f;

	// player hunger level (0 to 100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Hunger = 100.0f;

	// player stamina energy (0 to 100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Stamina = 100.0f;

	// tracks if the player has run out of health
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Stats")
	bool bIsDead = false;

	// --- resource inventory variables ---

	// wood inventory count
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	int32 Wood = 0;

	// stone inventory count
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	int32 Stone = 0;

	// berry inventory count
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	int32 Berry = 0;

	// array holding all resource counts (0=wood, 1=stone, 2=berry)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	TArray<int32> ResourcesArray;

	// --- building system variables ---

	// array holding crafted building counts (0=walls, 1=floors, 2=ceilings, 3=turrets)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	TArray<int32> BuildingArray;

	// true when currently positioning a building part
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	bool isBuilding = false;

	// class of building part to spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	TSubclassOf<ABuildingPart> BuildPartClass;

	// reference to player status hud widget
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player HUD")
	UPlayerWidget* PlayerUI;

	// reference to objective hud widget
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player HUD")
	UObjectiveWidget* ObjectWidget;

	// count of placed building parts towards objective
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objectives")
	float ObjectsBuilt = 0.0f;

	// count of total collected materials towards objective
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objectives")
	float MatsCollected = 0.0f;

	// pointer to building part currently being placed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	ABuildingPart* SpawnedPart;

	// list of resource names matching array indices
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	TArray<FString> ResourceNames;

	// decal material spawned when hitting resources
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	UMaterialInterface* HitDecal;

	// first person camera attached to character head
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* PlayerCameraComponent;

	// repeating timer handle for decreasing stats
	FTimerHandle StatsTimerHandle;
};
