// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

// We need this include so we can declare a UCameraComponent pointer below
#include "Camera/CameraComponent.h"

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

	// Will be used later to do a Line Trace and detect objects in front of the player
	// This gets triggered by the Left Mouse Button ("Interact" input)
	UFUNCTION()
	void FindObject();

	// Our first-person camera — this gets attached to the character's head
	// so the player can look down and see their own body
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* PlayerCameraComponent;

};
