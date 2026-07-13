// Fill out your copyright notice in the Description page of Project Settings.

#include "Resource_M.h"

// Constructor — creates the mesh and text components and hooks them together
AResource_M::AResource_M()
{
	// Allow this actor to tick every frame (we don't really need it, but it's on by default)
	PrimaryActorTick.bCanEverTick = true;

	// Create the static mesh component — this is the actual 3D shape you see in the world
	// You'll set the mesh (cube, tree, rock, etc.) in the Blueprint editor
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	// Make the mesh the root component so everything else attaches to it
	RootComponent = Mesh;

	// Create the text render component — this displays the resource name floating above the object
	// Super helpful for testing so you can tell what each resource is at a glance
	ResourceNameText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ResourceNameText"));

	// Attach the text to the mesh so it moves with the object
	ResourceNameText->SetupAttachment(Mesh);

	// Use absolute scale for the text so it doesn't shrink/grow when the mesh is resized
	// (false, false, true) = relative location, relative rotation, ABSOLUTE scale
	ResourceNameText->SetAbsolute(false, false, true);
}

// Called when the game starts — sets the floating text to show this resource's name
void AResource_M::BeginPlay()
{
	Super::BeginPlay();

	// Convert our FString resource name into FText (that's what TextRender needs)
	tempText = FText::FromString(resourceName);

	// Set the text component to display our resource name
	ResourceNameText->SetText(tempText);
}
