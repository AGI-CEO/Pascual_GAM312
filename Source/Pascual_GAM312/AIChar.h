// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AIChar.generated.h"

// This is our AI Character class — it's a simple Character that serves as
// the C++ base for our AI Blueprint. All of the actual wander behavior
// is set up inside the AI Controller Blueprint, not here in C++.
UCLASS()
class PASCUAL_GAM312_API AAIChar : public ACharacter
{
	GENERATED_BODY()

public:
	// Constructor — sets default values for this character
	AAIChar();

protected:
	// Called when the game starts or when this actor is spawned
	virtual void BeginPlay() override;
};
