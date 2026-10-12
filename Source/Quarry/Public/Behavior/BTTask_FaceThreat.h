// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FaceThreat.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Face Threat"))
class QUARRY_API UBTTask_FaceThreat : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_FaceThreat();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;
	
protected:
	UPROPERTY(EditAnywhere, Category = "Face")
	float TurnSpeed = 180.0f;
	
	UPROPERTY(EditAnywhere, Category = "Face")
	float AcceptableAngle = 5.0f;
};
