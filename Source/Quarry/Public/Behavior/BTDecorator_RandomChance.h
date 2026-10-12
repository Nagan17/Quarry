// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_RandomChance.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Random Chance"))
class QUARRY_API UBTDecorator_RandomChance : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTDecorator_RandomChance();
	
	virtual FString GetStaticDescription() const override;
	
protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	
	UPROPERTY(EditAnywhere, Category = "Condition", meta = (ClampMin = "0", ClampMax = "1"))
	float Chance = 0.6f;
};
