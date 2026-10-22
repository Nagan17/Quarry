// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "WolfPackSubsystem.generated.h"

class AWolf;

/**
 * 
 */
UCLASS()
class QUARRY_API UWolfPackSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	void RegisterWolf(AWolf* Wolf);
	void UnregisterWolf(AWolf* Wolf);
	
	int32 CallPack(AWolf* Caller, AActor* Target, const FVector& TargetLocation, float Radius, int32 MaxWolves);
	
	bool TryTakeAttackToken(AWolf* Wolf, int32 MaxAttackers);
	void ReleaseAttackToken(AWolf* Wolf);
	
private:
	void CleanUp();
	
	TArray<TWeakObjectPtr<AWolf>> Wolves;
	TArray<TWeakObjectPtr<AWolf>> Attackers;
};
