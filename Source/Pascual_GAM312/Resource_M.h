// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// We need these includes for the visible mesh and the floating text label
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"

#include "Resource_M.generated.h"

// This is our master Resource class — every collectible object (Tree, Rock, Bush)
// will be a child Blueprint of this class. It has a mesh, a text label, and
// variables that control how much resource it gives and how much is left.
UCLASS()
class PASCUAL_GAM312_API AResource_M : public AActor
{
	GENERATED_BODY()
	
public:	
	// Constructor — sets up the mesh and text components
	AResource_M();

protected:
	// Runs once when the game starts — we use this to set the text label
	virtual void BeginPlay() override;

public:	

	// --- Resource Settings (editable in the Details panel of each Blueprint) ---

	// The name of this resource (e.g. "Wood", "Stone", "Berry")
	// We check this name to figure out which inventory slot to add to
	UPROPERTY(EditAnywhere, Category = "Resources")
	FString resourceName = "Wood";

	// How much resource the player gets each time they hit this object
	UPROPERTY(EditAnywhere, Category = "Resources")
	int32 resourceAmount = 5;

	// The total amount of resource this object has before it's used up
	// Once this hits 0, the object gets destroyed
	UPROPERTY(EditAnywhere, Category = "Resources")
	int32 totalResource = 100;

	// Temporary text variable — we use this to convert our FString name
	// into FText so the TextRenderComponent can display it
	FText tempText;

	// --- Components ---

	// Floating text that shows the resource name above the object
	// This is mainly for testing so we can see what each object is
	UPROPERTY(VisibleAnywhere)
	UTextRenderComponent* ResourceNameText;

	// The visible 3D mesh for this resource (cube placeholder by default,
	// you set the actual mesh in the Blueprint like a tree or rock model)
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;
};
