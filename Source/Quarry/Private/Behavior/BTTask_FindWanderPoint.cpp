// Fill out your copyright notice in the Description page of Project Settings.


#include "Behavior/BTTask_FindWanderPoint.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Animals/AnimalAIController.h"
#include "Animals/BaseAnimal.h"
#include "Animals/AnimalData.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_FindWanderPoint::UBTTask_FindWanderPoint()
{
	NodeName = TEXT("Find Wander Point");
}

EBTNodeResult::Type UBTTask_FindWanderPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AIC = OwnerComp.GetAIOwner();
	const ABaseAnimal* Animal = AIC ? Cast<ABaseAnimal>(AIC->GetPawn()) : nullptr;
	const UAnimalData* Data = Animal ? Animal->GetAnimalData() : nullptr;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!Animal || !Data || !BB) return EBTNodeResult::Failed;
	
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(Animal->GetWorld());
	if (!NavSys) return EBTNodeResult::Failed;
	
	const FVector Home = BB->GetValueAsVector(AAnimalAIController::BB_HomeLocation);
	const FVector AnimalLoc = Animal->GetActorLocation();
	
	for (int32 Attempt = 0; Attempt < 5; Attempt++)
	{
		FNavLocation Result;
		if (NavSys->GetRandomReachablePointInRadius(Home, Data->WanderRadius, Result) &&
			FVector::Dist2D(Result.Location, AnimalLoc) >= Data->MinWanderDistance)
		{
			BB->SetValueAsVector(AAnimalAIController::BB_MoveTarget, Result.Location);
			return EBTNodeResult::Succeeded;
		}
	}
	
	return EBTNodeResult::Failed;
}
