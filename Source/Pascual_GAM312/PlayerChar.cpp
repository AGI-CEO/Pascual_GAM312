// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerChar.h"
#include "TurretBuildingPart.h"
#include "TimerManager.h"
#include "Engine/DamageEvents.h"

// ============================================================================
// camera use in game development:
// camera libraries in game engines convert 3d world coordinates into 2d screen
// pixels using view and projection matrices. attaching a camera component to the
// character head socket creates a true first-person perspective that moves with
// head animations and rotates with mouse input.
// ============================================================================

APlayerChar::APlayerChar()
{
	PrimaryActorTick.bCanEverTick = true;

	PlayerCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCameraComponent"));
	PlayerCameraComponent->SetupAttachment(GetMesh(), FName("head"));
	PlayerCameraComponent->bUsePawnControlRotation = true;

	ResourcesArray.SetNum(3);
	ResourceNames.SetNum(3);
	ResourceNames[0] = "Wood";
	ResourceNames[1] = "Stone";
	ResourceNames[2] = "Berry";

	BuildingArray.SetNum(4);
}

void APlayerChar::BeginPlay()
{
	Super::BeginPlay();

	// ensure resource and building arrays are properly initialized
	if (ResourcesArray.Num() < 3)
	{
		ResourcesArray.SetNum(3);
	}
	if (ResourceNames.Num() < 3)
	{
		ResourceNames.SetNum(3);
		ResourceNames[0] = "Wood";
		ResourceNames[1] = "Stone";
		ResourceNames[2] = "Berry";
	}
	if (BuildingArray.Num() < 4)
	{
		BuildingArray.SetNum(4);
	}

	GetWorld()->GetTimerManager().SetTimer(
		StatsTimerHandle,
		this,
		&APlayerChar::DecreaseStats,
		2.0f,
		true
	);

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
	// vector addition and scalar multiplication position building parts in
	// front of the player. CamLocation is a 3d point and CamForward is a unit
	// direction vector. multiplying by 400 extends the vector forward, and
	// adding CamLocation computes the world space placement point.
	// ========================================================================
	if (isBuilding && SpawnedPart)
	{
		FVector CamLocation = PlayerCameraComponent->GetComponentLocation();
		FVector CamForward = PlayerCameraComponent->GetForwardVector();
		FVector PlaceLocation = CamLocation + (CamForward * 400.0f);

		SpawnedPart->SetActorLocation(PlaceLocation);
	}

	if (PlayerUI)
	{
		PlayerUI->UpdateBars(Health, Hunger, Stamina);
	}

	// proximity melee attack check from nearby enemy AI characters
	static float MeleeHitCooldown = 0.0f;
	MeleeHitCooldown -= DeltaTime;

	if (MeleeHitCooldown <= 0.0f && Health > 0.0f)
	{
		TArray<AActor*> AllCharacters;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), AllCharacters);

		for (AActor* CharActor : AllCharacters)
		{
			if (CharActor && CharActor != this)
			{
				float Dist = FVector::Dist(GetActorLocation(), CharActor->GetActorLocation());
				if (Dist <= 180.0f)
				{
					SetHealth(-15.0f);
					MeleeHitCooldown = 1.5f;

					if (GEngine)
					{
						GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, FString::Printf(TEXT("Enemy AI attacked you! -15 HP (Health: %.0f/100)"), Health));
					}
					break;
				}
			}
		}
	}
}

void APlayerChar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerChar::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerChar::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);

	PlayerInputComponent->BindAction("JumpEvent", IE_Pressed, this, &APlayerChar::StartJump);
	PlayerInputComponent->BindAction("JumpEvent", IE_Released, this, &APlayerChar::StopJump);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &APlayerChar::FindObject);
	PlayerInputComponent->BindAction("RotPart", IE_Pressed, this, &APlayerChar::RotateBuilding);
}

// ============================================================================
// linear algebra in gaming (movement & rotation matrices):
// rotation matrices take the player look angle and convert it into directional
// basis vectors. the x axis gives forward and the y axis gives right.
// AddMovementInput scales these unit vectors to move the character.
// ============================================================================
void APlayerChar::MoveForward(float axisValue)
{
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetUnitAxis(EAxis::X);
	AddMovementInput(Direction, axisValue);
}

void APlayerChar::MoveRight(float axisValue)
{
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetUnitAxis(EAxis::Y);
	AddMovementInput(Direction, axisValue);
}

void APlayerChar::StartJump()
{
	Jump();
}

void APlayerChar::StopJump()
{
	StopJumping();
}

// ============================================================================
// player stat management:
// clamps health, hunger, and stamina between 0 and 100
// ============================================================================

void APlayerChar::SetHealth(float amount)
{
	Health = FMath::Clamp(Health + amount, 0.0f, 100.0f);
}

void APlayerChar::SetHunger(float amount)
{
	Hunger = FMath::Clamp(Hunger + amount, 0.0f, 100.0f);
}

void APlayerChar::SetStamina(float amount)
{
	Stamina = FMath::Clamp(Stamina + amount, 0.0f, 100.0f);
}

void APlayerChar::DecreaseStats()
{
	SetHunger(-1.0f);

	if (Hunger > 0.0f)
	{
		SetStamina(2.0f);
	}
	else
	{
		SetHealth(-2.0f);
	}
}

// eats one berry from inventory to refill hunger and stamina
void APlayerChar::EatBerry()
{
	// check if player has berries in inventory (either in ResourcesArray[2] or Berry variable)
	int32 CurrentBerries = (ResourcesArray.IsValidIndex(2) ? ResourcesArray[2] : 0);
	if (CurrentBerries <= 0 && Berry > 0)
	{
		CurrentBerries = Berry;
	}

	if (CurrentBerries > 0)
	{
		CurrentBerries -= 1;
		Berry = CurrentBerries;

		if (ResourcesArray.IsValidIndex(2))
		{
			ResourcesArray[2] = CurrentBerries;
		}

		// restore hunger and stamina
		SetHunger(25.0f);
		SetStamina(15.0f);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Green, FString::Printf(TEXT("Ate Berry! Hunger: %.0f/100, Stamina: %.0f/100 (Berries left: %d)"), Hunger, Stamina, CurrentBerries));
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

float APlayerChar::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	SetHealth(-ActualDamage);
	return ActualDamage;
}

// adds collected resources to inventory
void APlayerChar::GiveResource(int32 amount, FString resourceType)
{
	if (ResourcesArray.Num() < 3)
	{
		ResourcesArray.SetNum(3);
	}

	if (resourceType.Equals(TEXT("Wood"), ESearchCase::IgnoreCase))
	{
		ResourcesArray[0] += amount;
		Wood = ResourcesArray[0];
	}
	else if (resourceType.Equals(TEXT("Stone"), ESearchCase::IgnoreCase) || resourceType.Equals(TEXT("Rock"), ESearchCase::IgnoreCase))
	{
		ResourcesArray[1] += amount;
		Stone = ResourcesArray[1];
	}
	else if (resourceType.Equals(TEXT("Berry"), ESearchCase::IgnoreCase) || resourceType.Equals(TEXT("Berries"), ESearchCase::IgnoreCase))
	{
		ResourcesArray[2] += amount;
		Berry = ResourcesArray[2];
	}
}

// ============================================================================
// trace and collision in gaming:
// line tracing (raycasting) projects a ray from a start vector to an end vector.
// the physics engine tests bounding boxes and meshes along the ray using
// collision channels like ECC_Visibility, returning hit coordinates and actors.
// ============================================================================
void APlayerChar::FindObject()
{
	if (isBuilding)
	{
		isBuilding = false;

		if (SpawnedPart)
		{
			SpawnedPart->Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

			ATurretBuildingPart* TurretPart = Cast<ATurretBuildingPart>(SpawnedPart);
			if (TurretPart)
			{
				TurretPart->OnPlaced();
			}
		}

		ObjectsBuilt += 1.0f;

		if (ObjectWidget)
		{
			ObjectWidget->UpdateBuildObject(ObjectsBuilt);
		}

		return;
	}

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

			GiveResource(resourceValue, hitName);

			MatsCollected += static_cast<float>(resourceValue);
			if (ObjectWidget)
			{
				ObjectWidget->UpdateMatObjectives(MatsCollected);
			}

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

// checks resource requirements and crafts building parts
void APlayerChar::UpdateResources(int32 woodAmount, int32 stoneAmount, FString buildingObject)
{
	if (!ResourcesArray.IsValidIndex(0) || !ResourcesArray.IsValidIndex(1))
	{
		return;
	}

	// only craft if player has enough wood and stone
	if (ResourcesArray[0] >= woodAmount && ResourcesArray[1] >= stoneAmount)
	{
		ResourcesArray[0] -= woodAmount;
		Wood = ResourcesArray[0];

		ResourcesArray[1] -= stoneAmount;
		Stone = ResourcesArray[1];

		if (buildingObject == "Wall" && BuildingArray.IsValidIndex(0))
		{
			BuildingArray[0] += 1;
		}
		else if (buildingObject == "Floor" && BuildingArray.IsValidIndex(1))
		{
			BuildingArray[1] += 1;
		}
		else if (buildingObject == "Ceiling" && BuildingArray.IsValidIndex(2))
		{
			BuildingArray[2] += 1;
		}
		else if (buildingObject == "Turret" && BuildingArray.IsValidIndex(3))
		{
			BuildingArray[3] += 1;
		}

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("Crafted %s!"), *buildingObject));
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Not enough resources to craft!"));
		}
	}
}

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

		if (SpawnedPart)
		{
			SpawnedPart->Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		isSuccess = true;
		return;
	}

	isSuccess = false;
}

void APlayerChar::RotateBuilding()
{
	if (SpawnedPart)
	{
		SpawnedPart->AddActorLocalRotation(FRotator(0.0f, 90.0f, 0.0f));
	}
}
