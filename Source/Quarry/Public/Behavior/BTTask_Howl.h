// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Howl.generated.h"

struct FBTHowlMemory
{
	float TimeLeft = 0.f;
	bool bTurning = false;
};

/**
 * 
 */
UCLASS(meta = (DisplayName = "Howl"))
class QUARRY_API UBTTask_Howl : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_Howl();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual uint16 GetInstanceMemorySize() const override;
};
