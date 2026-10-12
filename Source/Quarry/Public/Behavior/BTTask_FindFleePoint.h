// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindFleePoint.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Find Flee Point"))
class QUARRY_API UBTTask_FindFleePoint : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_FindFleePoint();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
protected:
	UPROPERTY(EditAnywhere, Category = "Flee")
	float RandomAngleJitter = 15.0f;
	
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawDebug = true;
};
