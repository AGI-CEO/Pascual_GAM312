// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ObjectiveWidget.generated.h"

/**
 * 
 */
// This is our custom Widget class for tracking player objectives (materials collected and parts built).
// It inherits from UUserWidget, which is Unreal Engine's base class for on-screen UI elements.
// We declare UpdateMatObjectives and UpdateBuildObject here in C++ so we can call them from our Player Character,
// but the actual text updates and win condition logic happen inside the Widget Blueprint (Objective_W).
UCLASS()
class PASCUAL_GAM312_API UObjectiveWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// This event updates the material collection counter on our UI.
	// "BlueprintImplementableEvent" means we trigger it in C++, but construct the visual logic in Blueprint.
	// Pass in MatsCollected as a float representing total materials gathered so far.
	UFUNCTION(BlueprintImplementableEvent, Category = "Objectives")
	void UpdateMatObjectives(float MatsCollected);

	// This event updates the building placement counter on our UI.
	// "BlueprintImplementableEvent" means we trigger it in C++, but construct the visual logic in Blueprint.
	// Pass in ObjectsBuilt as a float representing total structures placed so far.
	UFUNCTION(BlueprintImplementableEvent, Category = "Objectives")
	void UpdateBuildObject(float ObjectsBuilt);
};
