// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animals/AnimalData.h"
#include "Materials/MaterialInterface.h"
#include "WolfData.generated.h"

class UAnimMontage;

USTRUCT(BlueprintType)
struct FWolfAttack
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	FName Name;

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (ClampMin = "0"))
	float MinRange = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (ClampMin = "0"))
	float MaxRange = 300.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (ClampMin = "0"))
	float Weight = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (ClampMin = "0"))
	float Damage = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (ClampMin = "0"))
	float HitTime = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (ClampMin = "0"))
	float LungeSpeed = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (ClampMin = "0"))
	float LungeUpSpeed = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (ClampMin = "0"))
	float Knockback = 200.0f;
};

USTRUCT(BlueprintType)
struct FWolfVariant
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variant")
	TObjectPtr<UMaterialInterface> BodyMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variant")
	TObjectPtr<UMaterialInterface> FurMaterial = nullptr;
};

/**
 * 
 */
UCLASS(BlueprintType)
class QUARRY_API UWolfData : public UAnimalData
{
	GENERATED_BODY()
	
public:
	//Pack
	UPROPERTY(EditDefaultsOnly, Category="Wolf|Pack")
	TObjectPtr<UAnimMontage> HowlMontage;
	
	UPROPERTY(EditDefaultsOnly, Category="Wolf|Pack", meta=(ClampMin=0))
	float PackCallRadius = 6000.0f;
	
	UPROPERTY(EditDefaultsOnly, Category="Wolf|Pack", meta=(ClampMin=0, ClampMax=5))
	int32 MaxPackCalls = 2;
	
	UPROPERTY(EditDefaultsOnly, Category="Wolf|Pack", meta=(ClampMin=0))
	float HowlCooldown = 90.0f;
	
	UPROPERTY(EditDefaultsOnly, Category="Wolf|Pack", meta=(ClampMin=0))
	float HowlTurnSpeed = 270.0;
	
	//Attack
	UPROPERTY(EditDefaultsOnly, Category="Wolf|Attack", meta = (TitleProperty = "Name"))
	TArray<FWolfAttack> Attacks;
	
	UPROPERTY(EditDefaultsOnly, Category="Wolf|Attack", meta = (ClampMin=0))
	float AttackReach = 120.0;
	
	UPROPERTY(EditDefaultsOnly, Category = "Wolf|Attack", meta = (ClampMin = "0"))
	float ApproachTimeout = 6.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Wolf|Attack", meta = (ClampMin = "1"))
	int32 MaxSimultaneousAttackers = 1;
	
	//Circle
	UPROPERTY(EditDefaultsOnly, Category = "Wolf|Circle", meta = (ClampMin = "0"))
	float CircleRadius = 700.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Wolf|Circle", meta = (ClampMin = "0"))
	float CircleSpeed = 400.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Wolf|Circle", meta = (ClampMin = "0"))
	float CircleDuration = 3.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Wolf|Circle", meta = (ClampMin = "0"))
	float CircleDeviation = 1.5f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Wolf|Circle", meta = (ClampMin = "5", ClampMax = "90"))
	float CircleStepDegrees = 35.0f;
	
	//Appearance
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	TArray<FWolfVariant> Variants;
};
