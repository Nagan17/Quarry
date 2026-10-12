// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animals/AnimalData.h"
#include "AnimalAnimInstance.generated.h"

class ABaseAnimal;
class UCharacterMovementComponent;
/**
 * 
 */
UCLASS()
class QUARRY_API UAnimalAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Animal|Locomotion")
	float Speed = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Animal|Locomotion")
	bool bIsMoving = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "Animal|Locomotion")
	bool bIsFalling = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "Animal|Awareness")
	EAwarenessState AwarenessState = EAwarenessState::Calm;
	
	UPROPERTY(BlueprintReadOnly, Category = "Animal|Awareness")
	bool bIsAlert = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "Animal|Awareness")
	bool bIsGrazing = false;
	
	UPROPERTY(EditDefaultsOnly, Category = "Animal|Locomotion")
	float MovingThreshold = 5.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Animal|Posture")
	bool bIsResting = false;

	UPROPERTY(BlueprintReadOnly, Category = "Animal|Posture")
	bool bIsSleeping = false;
	
private:
	UPROPERTY()
	TObjectPtr<ABaseAnimal> Animal;
	
	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> MoveComp;
	
};
