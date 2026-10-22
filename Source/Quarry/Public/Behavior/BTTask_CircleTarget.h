// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_CircleTarget.generated.h"

class AAIController;
class UWolfData;

struct FBTCircleMemory
{
	float TimeLeft = 0.0f;
	float AngleDeg = 0.0f;
	float Direction = 1.0f;
};

/**
 * 
 */
UCLASS(meta = (DisplayName = "Circle Target"))
class QUARRY_API UBTTask_CircleTarget : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_CircleTarget();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual uint16 GetInstanceMemorySize() const override;
	
private:
	static void MoveToNextPoint(AAIController* AIC, const AActor* Target, const UWolfData* Data, FBTCircleMemory* Memory);
};
