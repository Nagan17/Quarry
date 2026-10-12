// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Graze.generated.h"


struct FBTGrazeMemory
{
	float RemainingTime = 0.f;
};


/**
 * 
 */
UCLASS(meta = (DisplayName = "Graze"))
class QUARRY_API UBTTask_Graze : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_Graze();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;
	
protected:
	UPROPERTY(EditAnywhere, Category = "Graze", meta = (ClampMin = "0"))
	float Duration = 10.0f;
	
	UPROPERTY(EditAnywhere, Category = "Graze", meta = (ClampMin = "0"))
	float RandomDeviation = 4.0f;
};
