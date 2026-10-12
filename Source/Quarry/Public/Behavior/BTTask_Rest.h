// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Rest.generated.h"

struct FBTRestMemory
{
	float RestTimeLeft = 0.f;
	float TimeUntilSleep = 0.f;
	bool bWillSleep = false;
	bool bGettingUp = false;
};

/**
 * 
 */
UCLASS(meta = (DisplayName = "Rest"))
class QUARRY_API UBTTask_Rest : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_Rest();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;
	
protected:
	UPROPERTY(EditAnywhere, Category = "Rest", meta = (ClampMin = "0"))
	float RestDuration = 30.0f;
	
	UPROPERTY(EditAnywhere, Category = "Rest", meta = (ClampMin = "0"))
	float RestDeviation = 15.0f;
	
	UPROPERTY(EditAnywhere, Category = "Rest|Sleep", meta = (ClampMin = "0", ClampMax = "1"))
	float SleepChance = 0.5f;
	
	UPROPERTY(EditAnywhere, Category = "Rest|Sleep", meta = (ClampMin = "0"))
	float SleepDelayMin = 5.0f;
	
	UPROPERTY(EditAnywhere, Category = "Rest|Sleep", meta = (ClampMin = "0"))
	float SleepDelayMax = 12.0f;
	
	
};
