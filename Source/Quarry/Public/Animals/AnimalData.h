// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AnimalData.generated.h"

class UBehaviorTree;

UENUM(BlueprintType)
enum class EAnimalCatagory : uint8
{
	Prey,
	Predator,
	Small
};

UENUM(BlueprintType)
enum class EAwarenessState : uint8
{
	Calm,
	Suspicious,
	Alarmed
};

UENUM(BlueprintType)
enum class EAnimalPosture : uint8
{
	Standing,
	LyingDown,
	Resting,
	Sleeping,
	GettingUp
};

class UAnimMontage;
class UBehaviorTree;
class UBloodData;

/**
 * 
 */
UCLASS(BlueprintType)
class QUARRY_API UAnimalData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "Animal")
	EAnimalCatagory Catagory = EAnimalCatagory::Prey;
	
	UPROPERTY(EditDefaultsOnly, Category = "Animal")
	TObjectPtr<UBehaviorTree> BehaviourTree;
	
	//Movement
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float WalkSpeed = 150.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float RunSpeed = 750.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float WanderRadius = 1500.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float MinWanderDistance = 300.0f;
	
	//Sight
	UPROPERTY(EditDefaultsOnly, Category = "Senses|Sight")
	float SightRadius = 2500.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Senses|Sight")
	float LoseSightRadius = 3000.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Senses|Sight")
	float PeripheralVisionAngle = 80.0f;
	
	//Hearing
	UPROPERTY(EditDefaultsOnly, Category = "Senses|Hearing")
	float HearingRange = 3000.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Senses|Hearing")
	float NoiseAwarenessScale = 0.5f;
	
	//Awareness
	UPROPERTY(EditDefaultsOnly, Category = "Awareness")
	float SightAwarenessPerSecond = 0.35f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Awareness")
	float CloseSightMultiplier = 2.5f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Awareness")
	float FarSightMultiplier = 0.3f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Awareness")
	float CrouchSightMultiplier = 0.4f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Awareness")
	float StillSightMultiplier = 0.1f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Awareness")
	float StillDetectRadius = 600.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Awareness")
	float AwarenessDecayPerSecond = 0.15f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Awareness")
	float CalmDownDelay = 2.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Awareness", meta = (ClampMin = "0", ClampMax = "1"))
	float SuspiciousThreshold  = 0.4f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Awareness", meta = (ClampMin = "0", ClampMax = "1"))
	float AlarmedThreshold = 0.8f;
	
	//Flee
	UPROPERTY(EditDefaultsOnly, Category = "Flee")
	float FleeDistance = 1500.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Flee")
	float FleeCooldown = 4.0f;
	
	//Rest
	UPROPERTY(EditDefaultsOnly, Category = "Rest")
	TObjectPtr<UAnimMontage> LieDownMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Rest")
	TObjectPtr<UAnimMontage> GetUpFromRestMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Rest")
	TObjectPtr<UAnimMontage> GetUpFromSleepMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = "Rest")
	float RestingSightMultiplier = 0.7f;

	UPROPERTY(EditDefaultsOnly, Category = "Rest")
	float SleepingSightMultiplier = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Rest")
	float SleepingHearingMultiplier = 0.4f;
	
	//Hurt
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hurt|Animation")
	TSoftObjectPtr<UAnimMontage> HurtMontage;
	
	UPROPERTY(EditDefaultsOnly, Category="Hurt")
	TObjectPtr<UBloodData> Blood;
	
	//Death
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Death|Animation")
	TSoftObjectPtr<UAnimMontage> DeathMontage;
	
	//Bone
	UPROPERTY(EditDefaultsOnly, Category = "Bone")
	FName FallbackBoneName = "";
};
