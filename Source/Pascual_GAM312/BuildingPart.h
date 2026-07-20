// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// We need the ArrowComponent so we can have a visible pivot point in the editor
#include "Components/ArrowComponent.h"

// We need the StaticMeshComponent so each building part can have a visible shape
#include "Components/StaticMeshComponent.h"

#include "BuildingPart.generated.h"

// This is the base class for all building parts (walls, floors, ceilings).
// Each building part has a mesh (the visible shape) and an arrow (the pivot point).
// We create child Blueprints from this class for each specific building type.
UCLASS()
class PASCUAL_GAM312_API ABuildingPart : public AActor
{
	GENERATED_BODY()
	
public:	
	// Constructor — sets up the mesh and arrow components
	ABuildingPart();

	// The 3D mesh that represents this building part (wall, floor, ceiling)
	// Set this in each child Blueprint to the correct shape
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Building")
	UStaticMeshComponent* Mesh;

	// The arrow component acts as our pivot point for rotation
	// All building parts share the same pivot location so they line up correctly
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Building")
	UArrowComponent* PivotArrow;
};
