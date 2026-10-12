// Fill out your copyright notice in the Description page of Project Settings.


#include "Behavior/BTDecorator_RandomChance.h"

UBTDecorator_RandomChance::UBTDecorator_RandomChance()
{
	NodeName = TEXT("Random Chance");

	bAllowAbortLowerPri = false;
	bAllowAbortChildNodes = false;
}

FString UBTDecorator_RandomChance::GetStaticDescription() const
{
	return FString::Printf(TEXT("%.0f%% chance"), Chance * 100.f);
}

bool UBTDecorator_RandomChance::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	return FMath::FRand() < Chance;
}
