// Fill out your copyright notice in the Description page of Project Settings.


#include "Behavior/BTTask_SetAnimalSpeed.h"
#include "AIController.h"
#include "Animals/BaseAnimal.h"

UBTTask_SetAnimalSpeed::UBTTask_SetAnimalSpeed()
{
	NodeName = TEXT("Set Animal Speed");
}

EBTNodeResult::Type UBTTask_SetAnimalSpeed::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AIC = OwnerComp.GetAIOwner();
	ABaseAnimal* Animal = AIC ? Cast<ABaseAnimal>(AIC->GetPawn()) : nullptr;
	if (!Animal) return EBTNodeResult::Failed;
	
	Animal->SetRunning(bRun);
	return EBTNodeResult::Succeeded;
}

FString UBTTask_SetAnimalSpeed::GetStaticDescription() const
{
	return bRun ? TEXT("Run") : TEXT("Walk");
}
