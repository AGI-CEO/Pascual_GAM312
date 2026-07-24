// Fill out your copyright notice in the Description page of Project Settings.

#include "AIChar.h"

// Constructor — sets default values
AAIChar::AAIChar()
{
	// We don't need Tick for this AI since the wander logic
	// is handled by the AI Controller Blueprint, not C++
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts
void AAIChar::BeginPlay()
{
	// Always call the parent version first so Unreal can do its setup
	Super::BeginPlay();
}
