// Fill out your copyright notice in the Description page of Project Settings.


#include "Behavior/BTTask_Graze.h"

#include "AIController.h"
#include "Animals/BaseAnimal.h"

UBTTask_Graze::UBTTask_Graze()
{
	NodeName = TEXT("Graze");
	bNotifyTick = true;
	bNotifyTaskFinished = true;
}

static ABaseAnimal* GrazeTask_GetAnimal(const UBehaviorTreeComponent& OwnerComp)
{
	const AAIController* AIC = OwnerComp.GetAIOwner();
	return AIC ? Cast<ABaseAnimal>(AIC->GetPawn()) : nullptr;
}

EBTNodeResult::Type UBTTask_Graze::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ABaseAnimal* Animal = GrazeTask_GetAnimal(OwnerComp);
	if (!Animal) return EBTNodeResult::Failed;
	
	FBTGrazeMemory* Memory = CastInstanceNodeMemory<FBTGrazeMemory>(NodeMemory);
	Memory->RemainingTime = FMath::Max(0.5f, Duration + FMath::FRandRange(-RandomDeviation, RandomDeviation));
	
	Animal->SetGrazing(true);
	return EBTNodeResult::InProgress;
}

void UBTTask_Graze::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTGrazeMemory* Memory = CastInstanceNodeMemory<FBTGrazeMemory>(NodeMemory);
	Memory->RemainingTime -= DeltaSeconds;
	
	if (Memory->RemainingTime <= 0.0f)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

void UBTTask_Graze::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	if (ABaseAnimal* Animal = GrazeTask_GetAnimal(OwnerComp))
	{
		Animal->SetGrazing(false);
	}
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

uint16 UBTTask_Graze::GetInstanceMemorySize() const
{
	return sizeof(FBTGrazeMemory);
}

FString UBTTask_Graze::GetStaticDescription() const
{
	return FString::Printf(TEXT("Graze %.1f +/- %.1f s"), Duration, RandomDeviation);
}
