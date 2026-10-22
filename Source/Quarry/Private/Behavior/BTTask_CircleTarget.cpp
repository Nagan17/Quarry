// Fill out your copyright notice in the Description page of Project Settings.


#include "Behavior/BTTask_CircleTarget.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "Animals/AnimalAIController.h"
#include "Animals/Wolf.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Datas/WolfData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_CircleTarget::UBTTask_CircleTarget()
{
	NodeName = TEXT("Circle Target");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_CircleTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	AWolf* Wolf = AIC ? Cast<AWolf>(AIC->GetPawn()) : nullptr;
	
	const UWolfData* Data = Wolf ? Wolf->GetWolfData() : nullptr;
	const UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	const AActor* Target = BB ? Cast<AActor>(BB->GetValueAsObject(AAnimalAIController::BB_ThreatActor)) : nullptr;
	
	if (!Wolf || !Target || !Data) return EBTNodeResult::Failed;
	
	Wolf->GetCharacterMovement()->MaxWalkSpeed = Data->CircleSpeed;
	
	FBTCircleMemory* Memory = CastInstanceNodeMemory<FBTCircleMemory>(NodeMemory);
	Memory->TimeLeft = FMath::Max(0.5f, Data->CircleDuration + FMath::FRandRange(-Data->CircleDeviation, Data->CircleDeviation));
	Memory->Direction = FMath::RandBool() ? 1.0f : -1.0f;
	
	const FVector FromTarget = Wolf->GetActorLocation() - Target->GetActorLocation();
	Memory->AngleDeg = FMath::RadiansToDegrees(FMath::Atan2(FromTarget.Y, FromTarget.X));
	
	MoveToNextPoint(AIC, Target, Data, Memory);
	return EBTNodeResult::InProgress;
}

void UBTTask_CircleTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	AWolf* Wolf = AIC ? Cast<AWolf>(AIC->GetPawn()) : nullptr;
	
	const UWolfData* Data = Wolf ? Wolf->GetWolfData() : nullptr;
	const UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	const AActor* Target = BB ? Cast<AActor>(BB->GetValueAsObject(AAnimalAIController::BB_ThreatActor)) : nullptr;
	
	if (!Wolf || !Data || !Target)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	FBTCircleMemory* Memory = CastInstanceNodeMemory<FBTCircleMemory>(NodeMemory);
	Memory->TimeLeft -= DeltaSeconds;
	if (Memory->TimeLeft <= 0.0f)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
	
	if (AIC->GetMoveStatus() == EPathFollowingStatus::Type::Idle)
	{
		MoveToNextPoint(AIC, Target, Data, Memory);
	}
}

void UBTTask_CircleTarget::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	if (TaskResult == EBTNodeResult::Aborted)
	{
		if (AAIController* AIC = OwnerComp.GetAIOwner())
		{
			AIC->StopMovement();
		}
	}
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

uint16 UBTTask_CircleTarget::GetInstanceMemorySize() const
{
	return sizeof(FBTCircleMemory);
}

void UBTTask_CircleTarget::MoveToNextPoint(AAIController* AIC, const AActor* Target, const UWolfData* Data,
	FBTCircleMemory* Memory)
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(AIC->GetWorld());
	if (!NavSys) return;
	
	for (int Try = 0; Try < 3; Try++)
	{
		Memory->AngleDeg += Data->CircleStepDegrees * Memory->Direction;
		
		const FVector Dir = FRotator(0.0f, Memory->AngleDeg, 0.0f).Vector();
		const FVector Point = Target->GetActorLocation() + Dir * Data->CircleRadius;
		
		FNavLocation Projected;
		if (!NavSys->ProjectPointToNavigation(Point, Projected, FVector(200.0f, 200.0f, 200.0f))) continue;
		
		if (AIC->MoveToLocation(Projected.Location, 50.0f, false) != EPathFollowingRequestResult::Failed)
		{
			return;
		}
	}
	
	Memory->Direction *= -1.0f;
}
