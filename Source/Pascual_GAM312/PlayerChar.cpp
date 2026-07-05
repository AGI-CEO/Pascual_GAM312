// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerChar.h"

// Constructor — runs when the character is first created (before the game even starts)
APlayerChar::APlayerChar()
{
 	// Allow this character to run Tick() every frame (we can disable this later for performance)
	PrimaryActorTick.bCanEverTick = true;

	// Create the camera component and give it a name that shows up in the editor
	PlayerCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCameraComponent"));

	// Attach the camera to the skeletal mesh's "head" socket
	// This way it moves with the character's head, giving us a true first-person perspective
	// where the player can look down and see their own body
	PlayerCameraComponent->SetupAttachment(GetMesh(), FName("head"));

	// Make the camera rotate with the player's controller input (mouse movement)
	// Without this, moving the mouse wouldn't actually rotate the camera view
	PlayerCameraComponent->bUsePawnControlRotation = true;

}

// Called once when the game starts — good place to initialize runtime stuff
void APlayerChar::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame — DeltaTime tells us how much time passed since the last frame
void APlayerChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// This is where we connect our input mappings (from Project Settings) to our C++ functions
void APlayerChar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// --- Axis Inputs (these fire every frame and pass in a float value) ---

	// W/S keys — calls our MoveForward function with +1 or -1
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerChar::MoveForward);

	// A/D keys — calls our MoveRight function with -1 or +1
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerChar::MoveRight);

	// Mouse Y axis — we use the built-in AddControllerPitchInput so we don't need a custom function
	// This handles looking up and down
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	// Mouse X axis — same idea, built-in AddControllerYawInput handles looking left and right
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);

	// --- Action Inputs (these fire once on key press/release) ---

	// When Space Bar is pressed, start jumping
	PlayerInputComponent->BindAction("JumpEvent", IE_Pressed, this, &APlayerChar::StartJump);

	// When Space Bar is released, stop jumping (so we don't jump forever)
	PlayerInputComponent->BindAction("JumpEvent", IE_Released, this, &APlayerChar::StopJump);

}

// Moves the character forward or backward based on where they're currently looking
void APlayerChar::MoveForward(float axisValue)
{
	// Build a rotation matrix from the controller's current rotation (where we're looking),
	// then grab the forward direction vector (X axis) from it
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetUnitAxis(EAxis::X);

	// Apply movement in that direction — axisValue of +1 moves forward, -1 moves backward
	AddMovementInput(Direction, axisValue);
}

// Moves the character left or right based on where they're currently looking
void APlayerChar::MoveRight(float axisValue)
{
	// Same idea as MoveForward, but we grab the right direction vector (Y axis) instead
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetUnitAxis(EAxis::Y);

	// Apply movement — axisValue of +1 strafes right, -1 strafes left
	AddMovementInput(Direction, axisValue);
}

// Triggers the jump — ACharacter already has a built-in Jump() function, so we just call it
void APlayerChar::StartJump()
{
	Jump();
}

// Stops the jump when the player lets go of the button
void APlayerChar::StopJump()
{
	StopJumping();
}

// This will eventually handle our Line Trace to detect objects in the world
// (e.g., items we can pick up, doors we can open, etc.)
// For now, it's just an empty stub — we'll fill it in later
void APlayerChar::FindObject()
{

}
