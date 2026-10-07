// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BaseAnimInstance.generated.h"

/**
 * 
 */

class ABaseCharacter;
class UCharacterMovementComponent;

UCLASS()
class QUARRY_API UBaseAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
protected:
	
	UPROPERTY(BlueprintReadOnly, Category = "References")
	TObjectPtr<ABaseCharacter> Character;
	
	UPROPERTY(BlueprintReadOnly, Category = "References")
	TObjectPtr<UCharacterMovementComponent> MovementComponent;
	
	//Locomotion
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	FVector Velocity = FVector::ZeroVector;
 
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float GroundSpeed = 0.f;
 
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bShouldMove = false;
 
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bIsFalling = false;
 
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float Direction = 0.f;
	
	//Combat
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsAiming = false;
 
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsArmed = false;
 
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float AimPitch = 0.f;
 
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float AimYaw = 0.f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float RifleUpperBodyWeight = 1.f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Combat|IK")
	FTransform LeftHandIKTransform;
	
	UPROPERTY(BlueprintReadOnly, Category = "Combat|IK")
	FVector LeftHandIKLocation = FVector::ZeroVector;
	
	UPROPERTY(BlueprintReadOnly, Category = "Combat|IK")
	FRotator LeftHandIKRotation = FRotator::ZeroRotator;
};
