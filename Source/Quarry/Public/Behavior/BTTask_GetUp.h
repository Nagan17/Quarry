// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_GetUp.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Get Up"))
class QUARRY_API UBTTask_GetUp : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_GetUp();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
