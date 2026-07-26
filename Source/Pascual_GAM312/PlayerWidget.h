// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerWidget.generated.h"

// This is our custom Widget class for the player's HUD.
// It inherits from UUserWidget, which is Unreal's base class for on-screen UI elements.
// We declare UpdateBars here in C++ so we can call it from the Player Character,
// but the actual progress bar wiring happens in the Widget Blueprint.
UCLASS()
class PASCUAL_GAM312_API UPlayerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// This function gets called every frame from PlayerChar to update the HUD bars.
	// "BlueprintImplementableEvent" means we declare it here in C++,
	// but we build the actual logic (setting progress bar percentages) in the Blueprint.
	// health, hunger, and stamina are passed in as floats from 0 to 100.
	UFUNCTION(BlueprintImplementableEvent, Category = "Player HUD")
	void UpdateBars(float Health1, float Hunger1, float Stamina1);
};
