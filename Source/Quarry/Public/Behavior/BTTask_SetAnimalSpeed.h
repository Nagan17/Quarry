// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SetAnimalSpeed.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Set Animal Speed"))
class QUARRY_API UBTTask_SetAnimalSpeed : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_SetAnimalSpeed();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	
protected:
	UPROPERTY(EditAnywhere, Category = "Animal")
	bool bRun = false;
};
