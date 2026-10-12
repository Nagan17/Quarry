// Fill out your copyright notice in the Description page of Project Settings.


#include "Animals/BaseAnimal.h"

#include "Animals/AnimalAIController.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ABaseAnimal::ABaseAnimal()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	AIControllerClass = AAnimalAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	bUseControllerRotationYaw = false;
	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->bOrientRotationToMovement = true;
	Move->bUseControllerDesiredRotation = false;
	Move->RotationRate = FRotator(0, 360, 0);
}

void ABaseAnimal::SetRunning(bool bRun)
{
	if (!AnimalData) return;
	GetCharacterMovement()->MaxWalkSpeed = bRun ? AnimalData->RunSpeed : AnimalData->WalkSpeed;
}

// Called when the game starts or when spawned
void ABaseAnimal::BeginPlay()
{
	Super::BeginPlay();
	SetRunning(false);
}

