// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerChar.h"

// We need this include so we can use GetWorld()->GetTimerManager()
#include "TimerManager.h"

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

	// --- Set up our Resource Arrays ---
	// We use arrays so we can manage all resources in one place
	// Index 0 = Wood, Index 1 = Stone, Index 2 = Berry

	// Create 3 empty slots in the resources array (all start at 0)
	ResourcesArray.SetNum(3);

	// Set up the name array so we know which index is which resource
	ResourceNames.SetNum(3);
	ResourceNames[0] = "Wood";
	ResourceNames[1] = "Stone";
	ResourceNames[2] = "Berry";

	// Create 3 empty slots in the building array (all start at 0)
	// Index 0 = Walls, Index 1 = Floors, Index 2 = Ceilings
	BuildingArray.SetNum(3);
}

// Called once when the game starts — good place to initialize runtime stuff
void APlayerChar::BeginPlay()
{
	Super::BeginPlay();

	// Start a repeating timer that calls DecreaseStats() every 2 seconds
	// This makes hunger go down over time and handles stamina regen
	// "this" means we're calling a function on this player character
	GetWorld()->GetTimerManager().SetTimer(
		StatsTimerHandle,       // The timer handle we declared in the header
		this,                   // The object that owns the function (our player)
		&APlayerChar::DecreaseStats, // The function to call every tick
		2.0f,                   // How often to call it (every 2 seconds)
		true                    // true = keep repeating, false = only fire once
	);
}

// Called every frame — DeltaTime tells us how much time passed since the last frame
void APlayerChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// --- Building Placement Preview ---
	// When we're in building mode and have a valid spawned part,
	// move it to stay 400 units in front of the camera so it follows where we look
	if (isBuilding && SpawnedPart)
	{
		// Get where the camera is and which way it's pointing
		FVector CamLocation = PlayerCameraComponent->GetComponentLocation();
		FVector CamForward = PlayerCameraComponent->GetForwardVector();

		// Calculate a point 400 units ahead of the camera
		FVector PlaceLocation = CamLocation + (CamForward * 400.0f);

		// Move the building part to that point so it follows our view
		SpawnedPart->SetActorLocation(PlaceLocation);
	}
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

	// When Left Mouse Button is pressed, do a line trace to find and interact with objects
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &APlayerChar::FindObject);

	// When the E key is pressed, rotate the building part we're holding by 90 degrees
	PlayerInputComponent->BindAction("RotPart", IE_Pressed, this, &APlayerChar::RotateBuilding);
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

// ============================================================================
// PLAYER STAT FUNCTIONS
// These functions adjust our player's Health, Hunger, and Stamina values.
// We clamp them so they never go above 100 (but they can go below 0).
// Pass in a positive number to increase, or a negative number to decrease.
// ============================================================================

// Adds (or subtracts) from the player's health
// Won't let health go above 100
void APlayerChar::SetHealth(float amount)
{
	// Only add if the result would be under 100 — prevents overheal
	if (Health + amount < 100)
	{
		Health += amount;
	}
}

// Adds (or subtracts) from the player's hunger
// Won't let hunger go above 100
void APlayerChar::SetHunger(float amount)
{
	// Only add if the result would be under 100 — prevents overfeeding
	if (Hunger + amount < 100)
	{
		Hunger += amount;
	}
}

// Adds (or subtracts) from the player's stamina
// Won't let stamina go above 100
void APlayerChar::SetStamina(float amount)
{
	// Only add if the result would be under 100 — prevents going over max
	if (Stamina + amount < 100)
	{
		Stamina += amount;
	}
}

// This runs every 2 seconds on a timer (set up in BeginPlay)
// It handles:
//   1. Hunger slowly going down over time
//   2. Stamina slowly regenerating over time (if not starving)
//   3. Health going down when hunger reaches 0
void APlayerChar::DecreaseStats()
{
	// Subtract 1 from hunger every 2 seconds — the player is always getting hungrier
	SetHunger(-1.0f);

	// If the player still has food in their belly, regenerate some stamina
	if (Hunger > 0)
	{
		SetStamina(2.0f);
	}

	// If hunger hit 0 or below, start taking damage — you're starving!
	if (Hunger <= 0)
	{
		SetHealth(-1.0f);
	}
}

// ============================================================================
// RESOURCE COLLECTION
// GiveResource adds resources to the right inventory slot based on the name.
// FindObject does a line trace from the camera to detect and collect resources.
// ============================================================================

// Adds resources to the correct inventory slot based on the resource type name
// "Wood" goes to index 0, "Stone" goes to index 1, "Berry" goes to index 2
void APlayerChar::GiveResource(int32 amount, FString resourceType)
{
	// Check which resource type we're collecting and add to the right slot
	if (resourceType == "Wood")
	{
		// Add to the Wood slot (index 0)
		ResourcesArray[0] += amount;
	}
	else if (resourceType == "Stone")
	{
		// Add to the Stone slot (index 1)
		ResourcesArray[1] += amount;
	}
	else if (resourceType == "Berry")
	{
		// Add to the Berry slot (index 2)
		ResourcesArray[2] += amount;
	}
}

// This is our main interaction function — it shoots a line trace forward from
// the camera and checks if we hit a resource object. If we did, it collects
// from it, spawns a decal, and destroys the resource when it's empty.
// BUT if we're in building mode, clicking will PLACE the building instead.
void APlayerChar::FindObject()
{
	// --- Building Mode Check ---
	// If we're currently placing a building, clicking should stop placement
	// instead of doing the resource trace
	if (isBuilding)
	{
		// We clicked while in building mode — place the part and stop building
		isBuilding = false;
		return;
	}

	// --- Normal Resource Collection ---
	// Only allow interaction if we have enough stamina (need at least 5)
	if (Stamina > 5)
	{
		// Subtract 5 stamina every time we swing/interact
		SetStamina(-5.0f);

		// --- Set up the Line Trace ---
		// A line trace is like shooting an invisible laser forward from the camera
		// to see what it hits

		// This will store info about whatever our trace hits
		FHitResult HitResult;

		// Start the trace from where the camera is in the world
		FVector StartLocation = PlayerCameraComponent->GetComponentLocation();

		// Get the direction the camera is facing and extend it 800 units out
		// (800 units is about how far the player can reach)
		FVector Direction = PlayerCameraComponent->GetForwardVector() * 800;

		// The end point is the start plus the direction — that's where the trace stops
		FVector EndLocation = StartLocation + Direction;

		// Set up collision query parameters for the trace
		FCollisionQueryParams QueryParams;

		// Tell the trace to ignore our own player — we don't want to hit ourselves!
		QueryParams.AddIgnoredActor(this);

		// Enable complex collision for more accurate hit detection
		QueryParams.bTraceComplex = true;

		// Allow face index to be returned (lets us get surface normal info if needed)
		QueryParams.bReturnFaceIndex = true;

		// Actually shoot the line trace into the world
		// We use the Visibility trace channel (ECC_Visibility) which is the default
		GetWorld()->LineTraceSingleByChannel(
			HitResult,            // Where to store the hit info
			StartLocation,        // Where the trace starts
			EndLocation,          // Where the trace ends
			ECC_Visibility,       // Which collision channel to use
			QueryParams           // Our collision settings (ignore self, etc.)
		);

		// --- Try to cast what we hit to a Resource ---
		// Cast means "check if this actor is actually a Resource_M type"
		// If it is, we get a pointer to it. If not, HitResource will be null.
		AResource_M* HitResource = Cast<AResource_M>(HitResult.GetActor());

		// Make sure we actually hit a valid resource (not a wall or the ground)
		// If we skip this check and hit something that's NOT a resource, the game crashes
		if (HitResource)
		{
			// Get the name of the resource we hit (e.g. "Wood", "Stone", "Berry")
			FString hitName = HitResource->resourceName;

			// Get how much this resource gives us per hit
			int32 resourceValue = HitResource->resourceAmount;

			// Always give the player the resources when they hit
			GiveResource(resourceValue, hitName);

			// Subtract the amount we're taking from the resource's total supply
			HitResource->totalResource = HitResource->totalResource - resourceValue;

			// Check if the resource still has stuff left
			if (HitResource->totalResource > 0)
			{
				// Resource still has supply — show collected message
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Resource Collected"));
				}
			}
			else
			{
				// Resource is empty — destroy the object and show depleted message
				HitResource->Destroy();

				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Resource Depleted"));
				}
			}

			// --- Spawn a Decal at the Hit Location ---
			// This creates a visual indicator (like a red circle) where we hit the resource
			// The decal only shows up if we assigned a material in the Blueprint
			if (HitDecal)
			{
				UGameplayStatics::SpawnDecalAtLocation(
					GetWorld(),                        // The world to spawn in
					HitDecal,                          // The decal material (set in Blueprint)
					FVector(10.0f, 10.0f, 10.0f),     // Size of the decal (10x10x10)
					HitResult.Location,                // Where the line trace hit
					FRotator(-90.0f, 0.0f, 0.0f),     // Rotation (face flat on surface)
					2.0f                               // How long the decal lasts (2 seconds)
				);
			}
		}
	}
}

// ============================================================================
// BUILDING SYSTEM FUNCTIONS
// These handle crafting items, spawning building parts, and rotating them.
// ============================================================================

// Subtracts the crafting cost from our resources and adds 1 to the correct building slot.
// woodAmount = how much wood this item costs to craft
// stoneAmount = how much stone this item costs to craft
// buildingObject = the name ("Wall", "Floor", or "Ceiling") to match the right slot
void APlayerChar::UpdateResources(int32 woodAmount, int32 stoneAmount, FString buildingObject)
{
	// Subtract the wood cost from our wood supply (index 0)
	ResourcesArray[0] -= woodAmount;

	// Subtract the stone cost from our stone supply (index 1)
	ResourcesArray[1] -= stoneAmount;

	// Figure out which building slot to add to based on the name string
	if (buildingObject == "Wall")
	{
		// Add 1 wall to our building inventory (index 0)
		BuildingArray[0] += 1;
	}
	else if (buildingObject == "Floor")
	{
		// Add 1 floor to our building inventory (index 1)
		BuildingArray[1] += 1;
	}
	else if (buildingObject == "Ceiling")
	{
		// Add 1 ceiling to our building inventory (index 2)
		BuildingArray[2] += 1;
	}
}

// Spawns a building part into the world 400 units ahead of the camera.
// buildingID = which slot to check (0=Wall, 1=Floor, 2=Ceiling)
// isSuccess = output — true if we spawned it, false if we don't have any
void APlayerChar::SpawnBuilding(int32 buildingID, bool& isSuccess)
{
	// Only spawn if we're not already placing something
	if (!isBuilding)
	{
		// Check if we actually have at least 1 of this building type
		if (BuildingArray[buildingID] > 0)
		{
			// We're now in building/placement mode
			isBuilding = true;

			// Calculate a position 400 units ahead of the camera to spawn at
			FVector CamLocation = PlayerCameraComponent->GetComponentLocation();
			FVector CamForward = PlayerCameraComponent->GetForwardVector();
			FVector SpawnLocation = CamLocation + (CamForward * 400.0f);

			// Start with zero rotation — the player can rotate with E later
			FRotator SpawnRotation = FRotator::ZeroRotator;

			// Set up spawn parameters (default settings are fine)
			FActorSpawnParameters SpawnParams;

			// Subtract 1 from the building slot since we're using it up
			BuildingArray[buildingID] -= 1;

			// Actually spawn the actor into the world
			// BuildPartClass is set from the Widget to the correct child BP
			SpawnedPart = GetWorld()->SpawnActor<ABuildingPart>(
				BuildPartClass,    // Which Blueprint class to spawn (Wall_BP, Floor_BP, etc.)
				SpawnLocation,     // Where to put it initially
				SpawnRotation,     // Starting rotation (zero)
				SpawnParams        // Extra spawn settings
			);

			// Tell the Widget that the spawn was successful
			isSuccess = true;
			return;
		}
	}

	// If we get here, either we're already building or we don't have any — spawn failed
	isSuccess = false;
}

// Rotates the building part we're currently holding by 90 degrees.
// This is bound to the E key so the player can orient walls and floors.
void APlayerChar::RotateBuilding()
{
	// Only rotate if we're actually holding a building part
	if (SpawnedPart)
	{
		// Add 90 degrees to the yaw (left-right rotation)
		// Pitch = 0 (no tilt), Yaw = 90 (turn), Roll = 0 (no roll)
		SpawnedPart->AddActorLocalRotation(FRotator(0.0f, 90.0f, 0.0f));
	}
}
