// Fill out your copyright notice in the Description page of Project Settings.

#include "BuildingPart.h"

// Constructor — creates our two components and hooks them together
ABuildingPart::ABuildingPart()
{
	// We don't need Tick for building parts — they just sit there after being placed
	PrimaryActorTick.bCanEverTick = false;

	// Create the arrow component — this is our pivot point for rotation
	// Making it the root means everything else rotates around it
	PivotArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("PivotArrow"));
	RootComponent = PivotArrow;

	// Create the mesh component — this is the visible wall/floor/ceiling shape
	// We attach it to the arrow so it moves and rotates with the pivot
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(PivotArrow);
}
