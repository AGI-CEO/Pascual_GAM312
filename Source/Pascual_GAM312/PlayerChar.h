// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

// We need this include so we can declare a UCameraComponent pointer below
#include "Camera/CameraComponent.h"

// Include our Resource master class so we can cast to it during line traces
#include "Resource_M.h"

// Include GameplayStatics so we can spawn decals when we hit resources
#include "Kismet/GameplayStatics.h"

#include "PlayerChar.generated.h"

// This is our main Player Character class — it inherits from ACharacter,
// which gives us built-in movement, collision, and animation support out of the box.
UCLASS()
class PASCUAL_GAM312_API APlayerChar : public ACharacter
{
	GENERATED_BODY()

public:
	// Constructor — this is where we set up default values and create components
	APlayerChar();

protected:
	// Runs once when the game starts or when this character is spawned into the world
	virtual void BeginPlay() override;

public:	
	// Runs every single frame — useful for things that need constant updating
	virtual void Tick(float DeltaTime) override;

	// This is where we hook up our keyboard/mouse inputs to actual functions
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Handles forward and backward movement (W/S keys)
	// The axisValue will be +1 for forward (W) or -1 for backward (S)
	UFUNCTION()
	void MoveForward(float axisValue);

	// Handles left and right strafing (A/D keys)
	// The axisValue will be -1 for left (A) or +1 for right (D)
	UFUNCTION()
	void MoveRight(float axisValue);

	// Called when the player presses the jump button (Space Bar)
	UFUNCTION()
	void StartJump();

	// Called when the player releases the jump button (Space Bar)
	UFUNCTION()
	void StopJump();

	// Does a Line Trace forward from the camera to detect and interact with resources
	// Gets triggered by the Left Mouse Button ("Interact" input)
	UFUNCTION()
	void FindObject();

	// --- Player Stat Functions ---
	// These let us change our stats from anywhere (Blueprints or C++)
	// Pass in a positive number to add, or a negative number to subtract

	// Adjusts the player's health by the given amount (won't go above 100)
	UFUNCTION(BlueprintCallable, Category = "Player Stats")
	void SetHealth(float amount);

	// Adjusts the player's hunger by the given amount (won't go above 100)
	UFUNCTION(BlueprintCallable, Category = "Player Stats")
	void SetHunger(float amount);

	// Adjusts the player's stamina by the given amount (won't go above 100)
	UFUNCTION(BlueprintCallable, Category = "Player Stats")
	void SetStamina(float amount);

	// Called on a timer every 2 seconds — decreases hunger over time,
	// regenerates stamina, and damages health if hunger hits 0
	UFUNCTION(BlueprintCallable, Category = "Player Stats")
	void DecreaseStats();

	// Adds collected resources to the right inventory slot based on type name
	// (e.g. "Wood" goes to index 0, "Stone" to index 1, "Berry" to index 2)
	UFUNCTION(BlueprintCallable, Category = "Resources")
	void GiveResource(int32 amount, FString resourceType);

	// --- Player Stat Variables ---
	// These track the player's current health, hunger, and stamina
	// All start at 100 and can go up or down during gameplay

	// How much health the player has — drops when hunger reaches 0
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Health = 100.0f;

	// How full the player is — goes down over time on its own
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Hunger = 100.0f;

	// How much energy the player has — used up when hitting resources,
	// but slowly regenerates over time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Stamina = 100.0f;

	// --- Resource Inventory Variables ---
	// These track how much of each resource the player has collected

	// How much wood the player has gathered
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	int32 Wood = 0;

	// How much stone the player has gathered
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	int32 Stone = 0;

	// How many berries the player has gathered
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	int32 Berry = 0;

	// Array that holds all resource counts in one place
	// Index 0 = Wood, Index 1 = Stone, Index 2 = Berry
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	TArray<int32> ResourcesArray;

	// Array of resource names that matches up with ResourcesArray
	// So ResourceNames[0] = "Wood" matches ResourcesArray[0] = wood count
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	TArray<FString> ResourceNames;

	// --- Decal Material ---
	// This is the material that spawns as a decal where our line trace hits
	// Set this in the Blueprint to the red hit decal material you create
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	UMaterialInterface* HitDecal;

	// Our first-person camera — this gets attached to the character's head
	// so the player can look down and see their own body
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* PlayerCameraComponent;

	// Timer handle for the DecreaseStats function — ticks every 2 seconds
	// to drain hunger, regen stamina, and apply damage if starving
	FTimerHandle StatsTimerHandle;
};
