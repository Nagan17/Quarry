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
};
