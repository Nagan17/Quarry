// Fill out your copyright notice in the Description page of Project Settings.


#include "Behavior/BTTask_Rest.h"
#include "AIController.h"
#include "Animals/BaseAnimal.h"

UBTTask_Rest::UBTTask_Rest()
{
	NodeName = TEXT("Rest");
	bNotifyTick = true;
}

static ABaseAnimal* RestTask_GetAnimal(const UBehaviorTreeComponent& OwnerComp)
{
	const AAIController* AIC = OwnerComp.GetAIOwner();
	return AIC ? Cast<ABaseAnimal>(AIC->GetPawn()) : nullptr;
}

EBTNodeResult::Type UBTTask_Rest::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ABaseAnimal* Animal = RestTask_GetAnimal(OwnerComp);
	if (!Animal || !Animal->IsStanding()) return EBTNodeResult::Failed;
	
	if (AAIController* AIC = OwnerComp.GetAIOwner())
	{
		AIC->StopMovement();
	}
	
	FBTRestMemory* Memory = CastInstanceNodeMemory<FBTRestMemory>(NodeMemory);
	Memory->RestTimeLeft = FMath::Max(1.0f, RestDuration + FMath::FRandRange(-RestDeviation, RestDeviation));
	Memory->bWillSleep = FMath::FRand() < SleepChance;
	Memory->TimeUntilSleep = FMath::FRandRange(SleepDelayMin, FMath::Max(SleepDelayMin, SleepDelayMax));
	Memory->bGettingUp = false;
	
	Animal->StartLyingDown();
	return EBTNodeResult::InProgress;
}

void UBTTask_Rest::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	ABaseAnimal* Animal = RestTask_GetAnimal(OwnerComp);
	if (!Animal)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	FBTRestMemory* Memory = CastInstanceNodeMemory<FBTRestMemory>(NodeMemory);
	const EAnimalPosture Posture = Animal->GetPosture();
	
	if (Memory->bGettingUp)
	{
		if (Posture == EAnimalPosture::Standing)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
		return;
	}
	
	if (Posture == EAnimalPosture::Standing)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
	
	if (Posture == EAnimalPosture::LyingDown) return;
	
	Memory->RestTimeLeft -= DeltaSeconds;
	
	if (Memory->bWillSleep && Posture == EAnimalPosture::Resting)
	{
		Memory->TimeUntilSleep -= DeltaSeconds;
		if (Memory->TimeUntilSleep <= 0.0f)
		{
			Animal->FallAsleep();
		}
	}
	
	if (Memory->RestTimeLeft <= 0.0f)
	{
		Animal->StartGettingUp();
		Memory->bGettingUp = true;
	}
}

uint16 UBTTask_Rest::GetInstanceMemorySize() const
{
	return sizeof(FBTRestMemory);
}

FString UBTTask_Rest::GetStaticDescription() const
{
	return FString::Printf(TEXT("Rest %.0f +/- %.0f s, %.0f%% sleep"), RestDuration, RestDeviation, SleepChance * 100.f);
}
