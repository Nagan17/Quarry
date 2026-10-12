// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/AnimalAnimInstance.h"

#include "Animals/AnimalAIController.h"
#include "Animals/BaseAnimal.h"
#include "GameFramework/CharacterMovementComponent.h"

void UAnimalAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	Animal = Cast<ABaseAnimal>(TryGetPawnOwner());
	MoveComp = Animal ? Animal->GetCharacterMovement() : nullptr;
}

void UAnimalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (!Animal)
	{
		Animal = Cast<ABaseAnimal>(TryGetPawnOwner());
		MoveComp = Animal ? Animal->GetCharacterMovement() : nullptr;
	}
	
	if (!Animal || !MoveComp) return;
	
	Speed = MoveComp->Velocity.Size2D();
	bIsMoving = Speed > MovingThreshold;
	bIsFalling = MoveComp->IsFalling();
	
	if (const AAnimalAIController* AIC = Cast<AAnimalAIController>(Animal->GetController()))
	{
		AwarenessState = AIC->GetAwarenessState();
	}
	
	bIsAlert = (AwarenessState == EAwarenessState::Suspicious) && !bIsMoving;
}
