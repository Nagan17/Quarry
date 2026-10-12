// Fill out your copyright notice in the Description page of Project Settings.


#include "Behavior/BTTask_FindFleePoint.h"
#include "AIController.h"
#include "AssetTypeCategories.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Animals/AnimalAIController.h"
#include "Animals/BaseAnimal.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_FindFleePoint::UBTTask_FindFleePoint()
{
	NodeName = TEXT("Find Flee Point");
}

EBTNodeResult::Type UBTTask_FindFleePoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AIC = OwnerComp.GetAIOwner();
	ABaseAnimal* Animal = AIC ? Cast<ABaseAnimal>(AIC->GetPawn()) : nullptr;
	const UAnimalData* Data = Animal ? Animal->GetAnimalData() : nullptr;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!Animal || !Data || !BB) return EBTNodeResult::Failed;
	
	UWorld* World = Animal->GetWorld();
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	if (!NavSys) return EBTNodeResult::Failed;
	
	const FVector AnimalLoc = Animal->GetActorLocation();
	const FVector ThreatLoc = BB->GetValueAsVector(AAnimalAIController::BB_ThreatLocation);
	
	FVector Away = (AnimalLoc - ThreatLoc).GetSafeNormal2D();
	if (Away.IsNearlyZero())
	{
		Away = Animal->GetActorForwardVector();
	}
	
	static const float Angles[] = { 0.f, 30.f, -30.f, 60.f, -60.f, 90.f, -90.f };
	const FVector ProjectExtent(300, 300, 500);
	
	for (const float Angle : Angles)
	{
		const float Jitter = FMath::FRandRange(-RandomAngleJitter, RandomAngleJitter);
		const FVector Dir = Away.RotateAngleAxis(Angle + Jitter, FVector::UpVector);
		const FVector Candidate = AnimalLoc + Dir * Data->FleeDistance;
		
		FNavLocation Projected;
		if (!NavSys->ProjectPointToNavigation(Candidate, Projected, ProjectExtent)) continue;
		
		const UNavigationPath* Path = UNavigationSystemV1::FindPathToLocationSynchronously(World, AnimalLoc, Projected.Location, Animal);
		if (!Path || !Path->IsValid() || Path->IsPartial()) continue;
		
#if ENABLE_DRAW_DEBUG
		if (bDrawDebug)
		{
			DrawDebugSphere(World, Projected.Location, 40.f, 12, FColor::Red, false, 3.f);
			DrawDebugLine(World, AnimalLoc, Projected.Location, FColor::Red, false, 3.f, 0, 2.f);
		}
#endif
		
		BB->SetValueAsVector(AAnimalAIController::BB_MoveTarget, Projected.Location);
		return EBTNodeResult::Succeeded;
	}
	
	FNavLocation Fallback;
	if (NavSys->GetRandomReachablePointInRadius(AnimalLoc, Data->FleeDistance, Fallback))
	{
		BB->SetValueAsVector(AAnimalAIController::BB_MoveTarget, Fallback.Location);
		return EBTNodeResult::Succeeded;
	}
	
	return EBTNodeResult::Failed;
}
