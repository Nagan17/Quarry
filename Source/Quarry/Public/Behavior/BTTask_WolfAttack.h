// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_WolfAttack.generated.h"

/**
 * 
 */

struct FBTWolfAttackMemory
{
	float Timer = 0.f;
	int32 AttackIndex = INDEX_NONE;
	bool bAttacking = false;
	bool bHasToken = false;
};


UCLASS(meta = (DisplayName = "Wolf Attack"))
class QUARRY_API UBTTask_WolfAttack : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_WolfAttack();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual uint16 GetInstanceMemorySize() const override;
};
