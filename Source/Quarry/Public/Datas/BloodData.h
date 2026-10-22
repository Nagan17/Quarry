// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BloodData.generated.h"

/**
 * 
 */

class UNiagaraSystem;
class UMaterialInterface;

UCLASS(BlueprintType)
class QUARRY_API UBloodData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	//Hit
	UPROPERTY(EditDefaultsOnly, Category= "Blood|Hit")
	TObjectPtr<UNiagaraSystem> HitFX;
	
	UPROPERTY(EditDefaultsOnly, Category="Blood|Hit")
	bool bSpawnExitSpray = false;
	
	UPROPERTY(EditDefaultsOnly, Category="Blood|Hit", meta=(EditCondition="bSpawnExitSpray"))
	float ExitIntensityScale = 1.5f;
	
	UPROPERTY(EditDefaultsOnly, Category="Blood|Hit", meta=(EditCondition="bSpawnExitSpray"))
	float MaxBodyThickness = 150.f;

	UPROPERTY(EditDefaultsOnly, Category="Blood|Hit")
	float DamageForMinIntensity = 20.f;

	UPROPERTY(EditDefaultsOnly, Category="Blood|Hit")
	float DamageForMaxIntensity = 100.f;

	UPROPERTY(EditDefaultsOnly, Category="Blood|Hit")
	float MinIntensity = 0.6f;

	UPROPERTY(EditDefaultsOnly, Category="Blood|Hit")
	float MaxIntensity = 1.5f;
	
	//Bleed
	UPROPERTY(EditDefaultsOnly, Category="Blood|Bleed")
	float DamageToBleed = 0.015f;
	
	UPROPERTY(EditDefaultsOnly, Category= "Blood|Bleed")
	float BleedDuration = 10.0f;
	
	//Trail
	UPROPERTY(EditDefaultsOnly, Category= "Blood|Trail")
	TArray<TObjectPtr<UMaterialInterface>> TrailDecals;
	
	UPROPERTY(EditDefaultsOnly, Category= "Blood|Trail")
	FVector2D TrailDecalSize = FVector2D(8.0f, 22.0f);
	
	UPROPERTY(EditDefaultsOnly, Category= "Blood|Trail")
	float DropSpacing = 120.0f;
	
	UPROPERTY(EditDefaultsOnly, Category= "Blood|Trail")
	float IdleDropInterval = 1.5f;
	
	UPROPERTY(EditDefaultsOnly, Category= "Blood|Trail")
	float DropScatter = 20.0f;
	
	//Splash
	UPROPERTY(EditDefaultsOnly, Category="Blood|Splash")
	TArray<TObjectPtr<UMaterialInterface>> SplashDecals;
	
	UPROPERTY(EditDefaultsOnly, Category="Blood|Splash")
	float SplashDecalSize = 60.f;

	UPROPERTY(EditDefaultsOnly, Category="Blood|Splash")
	float SplashRollOffset = 0.f;
	
	//Death
	UPROPERTY(EditDefaultsOnly, Category= "Blood|Death")
	TObjectPtr<UMaterialInterface> PoolDecal;
	
	UPROPERTY(EditDefaultsOnly, Category="Blood|Death")
	float PoolSize = 80.f;
	
	UPROPERTY(EditDefaultsOnly, Category= "Blood|Death")
	float PoolDelay = 2.5f;
	
	UPROPERTY(EditDefaultsOnly, Category="Blood|Death", meta=(ClampMin="0.01", ClampMax="1"))
	float PoolStartScale = 0.15f;

	UPROPERTY(EditDefaultsOnly, Category="Blood|Death", meta=(ClampMin="0.1"))
	float PoolGrowTime = 8.f;
	
	//Lifetime
	UPROPERTY(EditDefaultsOnly, Category= "Blood|Decals")
	float DecalLifetime = 90.0f;
	
	UPROPERTY(EditDefaultsOnly, Category= "Blood|Decals")
	float DecalFadeTime = 5.0f;
	
	UPROPERTY(EditDefaultsOnly, Category= "Blood|Decals")
	int32 MaxDecals = 40;
};
