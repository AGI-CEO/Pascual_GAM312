// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerChar.h"
#include "TurretBuildingPart.h"
#include "TimerManager.h"
#include "Engine/DamageEvents.h"

// ============================================================================
// camera use in game development:
// camera libraries in game engines take 3d world coordinates and convert them
// into 2d screen pixels using view and projection matrices. attaching a camera
// component to the character mesh head socket creates a true first-person point
// of view where the camera tracks player head animations and mouse rotation.
// ============================================================================

APlayerChar::APlayerChar()
{
	// allow tick every frame for placement updates
	PrimaryActorTick.bCanEverTick = true;

	// create the first-person camera component
	PlayerCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCameraComponent"));

	// attach camera to head socket so view matches character head movement
	PlayerCameraComponent->SetupAttachment(GetMesh(), FName("head"));

	// enable pawn control rotation so mouse movement rotates the camera view
	PlayerCameraComponent->bUsePawnControlRotation = true;

	// set up 3 slots for resources: 0 = wood, 1 = stone, 2 = berry
	ResourcesArray.SetNum(3);
	ResourceNames.SetNum(3);
	ResourceNames[0] = "Wood";
	ResourceNames[1] = "Stone";
	ResourceNames[2] = "Berry";

	// set up 4 slots for building parts: 0 = wall, 1 = floor, 2 = ceiling, 3 = turret
	BuildingArray.SetNum(4);
}

void APlayerChar::BeginPlay()
{
	Super::BeginPlay();

	// timer calls DecreaseStats every 2 seconds to drain hunger and regenerate stamina
	GetWorld()->GetTimerManager().SetTimer(
		StatsTimerHandle,
		this,
		&APlayerChar::DecreaseStats,
		2.0f,
		true
	);

	// initialize objective counters on hud
	if (ObjectWidget)
	{
		ObjectWidget->UpdateMatObjectives(MatsCollected);
		ObjectWidget->UpdateBuildObject(ObjectsBuilt);
	}
}

void APlayerChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ========================================================================
	// linear algebra in gaming (preview placement):
	// we use vector addition and scalar multiplication to place an object in
	// front of the player. CamLocation is a point in 3d space (x,y,z), and
	// CamForward is a normalized direction vector (length of 1). multiplying
	// CamForward by 400 extends the vector 400 units, and adding it to
	// CamLocation gives the target 3d point in world space.
	// ========================================================================
	if (isBuilding && SpawnedPart)
	{
		FVector CamLocation = PlayerCameraComponent->GetComponentLocation();
		FVector CamForward = PlayerCameraComponent->GetForwardVector();
		FVector PlaceLocation = CamLocation + (CamForward * 400.0f);

		SpawnedPart->SetActorLocation(PlaceLocation);
	}

	// update hud bars every frame
	if (PlayerUI)
	{
		PlayerUI->UpdateBars(Health, Hunger, Stamina);
	}
}

void APlayerChar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// movement axis bindings
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerChar::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerChar::MoveRight);

	// mouse look axis bindings
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);

	// action bindings for jumping, interacting, and building rotation
	PlayerInputComponent->BindAction("JumpEvent", IE_Pressed, this, &APlayerChar::StartJump);
	PlayerInputComponent->BindAction("JumpEvent", IE_Released, this, &APlayerChar::StopJump);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &APlayerChar::FindObject);
	PlayerInputComponent->BindAction("RotPart", IE_Pressed, this, &APlayerChar::RotateBuilding);
}

// ============================================================================
// linear algebra in gaming (movement & rotation matrices):
// rotation matrices take the player look angle and convert it into directional
// basis vectors. the x axis gives the forward vector, and the y axis gives the
// right vector. AddMovementInput scales these unit vectors by axisValue to move
// the character in the proper world direction.
// ============================================================================
void APlayerChar::MoveForward(float axisValue)
{
	if (bIsDead) return;

	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetUnitAxis(EAxis::X);
	AddMovementInput(Direction, axisValue);
}

void APlayerChar::MoveRight(float axisValue)
{
	if (bIsDead) return;

	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetUnitAxis(EAxis::Y);
	AddMovementInput(Direction, axisValue);
}

void APlayerChar::StartJump()
{
	if (bIsDead) return;
	Jump();
}

void APlayerChar::StopJump()
{
	StopJumping();
}

// ============================================================================
// player stat management:
// clamps stats between 0 and 100 to prevent going over maximum or under zero
// ============================================================================

void APlayerChar::SetHealth(float amount)
{
	Health = FMath::Clamp(Health + amount, 0.0f, 100.0f);

	if (Health <= 0.0f && !bIsDead)
	{
		Die();
	}
}

void APlayerChar::SetHunger(float amount)
{
	Hunger = FMath::Clamp(Hunger + amount, 0.0f, 100.0f);
}

void APlayerChar::SetStamina(float amount)
{
	Stamina = FMath::Clamp(Stamina + amount, 0.0f, 100.0f);
}

// drains hunger every 2 seconds, regens stamina, or damages player if starving
void APlayerChar::DecreaseStats()
{
	if (bIsDead) return;

	// drain hunger gradually over time
	SetHunger(-1.0f);

	// regenerate stamina if player is not starving
	if (Hunger > 0.0f)
	{
		SetStamina(2.0f);
	}
	else
	{
		// player takes damage when starving at 0 hunger
		SetHealth(-2.0f);
	}
}

// eats one berry from inventory to restore hunger and stamina
void APlayerChar::EatBerry()
{
	if (bIsDead) return;

	// check if player has berries in inventory (index 2)
	if (ResourcesArray.IsValidIndex(2) && ResourcesArray[2] > 0)
	{
		ResourcesArray[2] -= 1;
		Berry = ResourcesArray[2];

		// restore hunger and stamina clamped to max 100
		SetHunger(25.0f);
		SetStamina(15.0f);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Ate berry: +25 hunger, +15 stamina"));
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("No berries in inventory!"));
		}
	}
}

// handles damage from external sources like ai attacks
float APlayerChar::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (!bIsDead)
	{
		SetHealth(-ActualDamage);
	}

	return ActualDamage;
}

// handles player death and triggers lose widget
void APlayerChar::Die()
{
	bIsDead = true;
	Health = 0.0f;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("Player died! Game Over."));
	}

	// call blueprint event to display win/lose screen
	OnPlayerDeath();
}

// adds collected resources to the matching inventory slot
void APlayerChar::GiveResource(int32 amount, FString resourceType)
{
	if (resourceType == "Wood")
	{
		ResourcesArray[0] += amount;
		Wood = ResourcesArray[0];
	}
	else if (resourceType == "Stone")
	{
		ResourcesArray[1] += amount;
		Stone = ResourcesArray[1];
	}
	else if (resourceType == "Berry")
	{
		ResourcesArray[2] += amount;
		Berry = ResourcesArray[2];
	}
}

// ============================================================================
// trace and collision in gaming:
// line tracing (raycasting) projects a ray from a start vector to an end vector
// through the physics scene. the physics engine tests bounding boxes and polygon
// meshes along the ray using collision channels like ECC_Visibility. when an
// intersection occurs, HitResult returns hit coordinates, surface normals, and
// actor pointers so we can collect resources accurately.
// ============================================================================
void APlayerChar::FindObject()
{
	if (bIsDead) return;

	// if placing a building, clicking places the object down
	if (isBuilding)
	{
		isBuilding = false;

		if (SpawnedPart)
		{
			SpawnedPart->Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

			// if placing a turret, activate its targeting and firing
			ATurretBuildingPart* TurretPart = Cast<ATurretBuildingPart>(SpawnedPart);
			if (TurretPart)
			{
				TurretPart->OnPlaced();
			}
		}

		// update building objective count
		ObjectsBuilt += 1.0f;

		if (ObjectWidget)
		{
			ObjectWidget->UpdateBuildObject(ObjectsBuilt);
		}

		return;
	}

	// resource harvesting costs 5 stamina per hit
	if (Stamina >= 5.0f)
	{
		SetStamina(-5.0f);

		FHitResult HitResult;
		FVector StartLocation = PlayerCameraComponent->GetComponentLocation();
		FVector Direction = PlayerCameraComponent->GetForwardVector() * 800.0f;
		FVector EndLocation = StartLocation + Direction;

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnFaceIndex = true;

		// cast line trace using visibility channel
		GetWorld()->LineTraceSingleByChannel(
			HitResult,
			StartLocation,
			EndLocation,
			ECC_Visibility,
			QueryParams
		);

		AResource_M* HitResource = Cast<AResource_M>(HitResult.GetActor());

		if (HitResource)
		{
			FString hitName = HitResource->resourceName;
			int32 resourceValue = HitResource->resourceAmount;

			// add resources to inventory
			GiveResource(resourceValue, hitName);

			// update total collected materials objective
			MatsCollected += static_cast<float>(resourceValue);
			if (ObjectWidget)
			{
				ObjectWidget->UpdateMatObjectives(MatsCollected);
			}

			// deduct resource pool and destroy node when depleted
			HitResource->totalResource -= resourceValue;

			if (HitResource->totalResource > 0)
			{
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Resource Collected"));
				}
			}
			else
			{
				HitResource->Destroy();
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Resource Depleted"));
				}
			}

			// spawn hit decal at hit point
			if (HitDecal)
			{
				UGameplayStatics::SpawnDecalAtLocation(
					GetWorld(),
					HitDecal,
					FVector(10.0f, 10.0f, 10.0f),
					HitResult.Location,
					FRotator(-90.0f, 0.0f, 0.0f),
					2.0f
				);
			}
		}
	}
}

// subtracts crafting materials and adds crafted item to building array
void APlayerChar::UpdateResources(int32 woodAmount, int32 stoneAmount, FString buildingObject)
{
	ResourcesArray[0] -= woodAmount;
	Wood = ResourcesArray[0];

	ResourcesArray[1] -= stoneAmount;
	Stone = ResourcesArray[1];

	if (buildingObject == "Wall")
	{
		BuildingArray[0] += 1;
	}
	else if (buildingObject == "Floor")
	{
		BuildingArray[1] += 1;
	}
	else if (buildingObject == "Ceiling")
	{
		BuildingArray[2] += 1;
	}
	else if (buildingObject == "Turret")
	{
		BuildingArray[3] += 1;
	}
}

// spawns building part 400 units in front of player for placement
void APlayerChar::SpawnBuilding(int32 buildingID, bool& isSuccess)
{
	if (!isBuilding && BuildingArray.IsValidIndex(buildingID) && BuildingArray[buildingID] > 0)
	{
		isBuilding = true;

		FVector CamLocation = PlayerCameraComponent->GetComponentLocation();
		FVector CamForward = PlayerCameraComponent->GetForwardVector();
		FVector SpawnLocation = CamLocation + (CamForward * 400.0f);
		FRotator SpawnRotation = FRotator::ZeroRotator;

		FActorSpawnParameters SpawnParams;
		BuildingArray[buildingID] -= 1;

		SpawnedPart = GetWorld()->SpawnActor<ABuildingPart>(
			BuildPartClass,
			SpawnLocation,
			SpawnRotation,
			SpawnParams
		);

		// disable collision during preview movement so it doesn't push the player
		if (SpawnedPart)
		{
			SpawnedPart->Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		isSuccess = true;
		return;
	}

	isSuccess = false;
}

// rotates currently held building part by 90 degrees yaw
void APlayerChar::RotateBuilding()
{
	if (SpawnedPart)
	{
		SpawnedPart->AddActorLocalRotation(FRotator(0.0f, 90.0f, 0.0f));
	}
}
