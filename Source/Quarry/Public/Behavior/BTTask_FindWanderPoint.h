// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindWanderPoint.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Find Wander Point"))
class QUARRY_API UBTTask_FindWanderPoint : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_FindWanderPoint();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
