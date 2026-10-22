// Fill out your copyright notice in the Description page of Project Settings.


#include "Behavior/BTTask_GetUp.h"
#include "AIController.h"
#include "Animals/BaseAnimal.h"

UBTTask_GetUp::UBTTask_GetUp()
{
	NodeName = TEXT("Get Up");
	bNotifyTick = true;
}

static ABaseAnimal* GetUpTask_GetAnimal(const UBehaviorTreeComponent& OwnerComp)
{
	const AAIController* AIC = OwnerComp.GetAIOwner();
	return AIC ? Cast<ABaseAnimal>(AIC->GetPawn()) : nullptr;
}

EBTNodeResult::Type UBTTask_GetUp::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ABaseAnimal* Animal = GetUpTask_GetAnimal(OwnerComp);
	if (!Animal) return EBTNodeResult::Failed;
	
	if (Animal->IsStanding()) return EBTNodeResult::Succeeded;
	
	Animal->StartGettingUp();
	return EBTNodeResult::InProgress;
}

void UBTTask_GetUp::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	const ABaseAnimal* Animal = GetUpTask_GetAnimal(OwnerComp);
	if (!Animal)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	if (Animal->IsStanding())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
