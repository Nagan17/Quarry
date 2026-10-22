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
	
	const float RawSpeed = MoveComp->Velocity.Size2D();
	
	Speed = (RawSpeed >= Speed) ? RawSpeed : FMath::FInterpTo(Speed, RawSpeed, DeltaSeconds, SpeedFalloffRate);
	bIsMoving = Speed > MovingThreshold * 4.f;
	bIsFalling = MoveComp->IsFalling();
	
	if (const AAnimalAIController* AIC = Cast<AAnimalAIController>(Animal->GetController()))
	{
		AwarenessState = AIC->GetAwarenessState();
	}
	
	bIsAlert = (AwarenessState == EAwarenessState::Suspicious) && !bIsMoving;
	bIsGrazing = Animal->IsGrazing();
	
	const EAnimalPosture Posture = Animal->GetPosture();
	bIsResting  = (Posture == EAnimalPosture::LyingDown || Posture == EAnimalPosture::Resting);
	bIsSleeping = (Posture == EAnimalPosture::Sleeping);
	
	if (const APawn* Pawn = TryGetPawnOwner())
	{
		const float Yaw = Pawn->GetActorRotation().Yaw;
		float TargetRate = 0.0f;

		if (bHasLastYaw && DeltaSeconds > KINDA_SMALL_NUMBER && Speed > MovingThreshold)
		{
			TargetRate = FMath::FindDeltaAngleDegrees(LastYaw, Yaw) / DeltaSeconds;
			TargetRate = FMath::Clamp(TargetRate, -MaxTurnRate, MaxTurnRate);
		}

		LastYaw = Yaw;
		bHasLastYaw = true;

		TurnRate = FMath::FInterpTo(TurnRate, TargetRate, DeltaSeconds, TurnRateInterpSpeed);
	}
}
