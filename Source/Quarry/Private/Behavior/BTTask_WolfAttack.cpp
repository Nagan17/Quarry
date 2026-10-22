// Fill out your copyright notice in the Description page of Project Settings.


#include "Behavior/BTTask_WolfAttack.h"

#include "AIController.h"
#include "Animals/AnimalAIController.h"
#include "Animals/Wolf.h"
#include "Animals/WolfPackSubsystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Datas/WolfData.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_WolfAttack::UBTTask_WolfAttack()
{
	NodeName = TEXT("Wolf Attack");
	bNotifyTick = true;
	bNotifyTaskFinished = true;
}

static AWolf* AttackTask_GetWolf(const UBehaviorTreeComponent& OwnerComp)
{
	const AAIController* AIC = OwnerComp.GetAIOwner();
	return AIC ? Cast<AWolf>(AIC->GetPawn()) : nullptr;
}

static AActor* AttackTask_GetTarget(const UBehaviorTreeComponent& OwnerComp)
{
	const UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	return BB ? Cast<AActor>(BB->GetValueAsObject(AAnimalAIController::BB_ThreatActor)) : nullptr;
}

static float AttackTask_ApproachRadius(const FWolfAttack& Attack)
{
	return FMath::Max(Attack.MinRange, Attack.MaxRange * 0.5f);
}

EBTNodeResult::Type UBTTask_WolfAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTWolfAttackMemory* Memory = CastInstanceNodeMemory<FBTWolfAttackMemory>(NodeMemory);
	Memory->bHasToken = false;
	Memory->bAttacking = false;
	Memory->AttackIndex = INDEX_NONE;
	
	AAIController* AIC = OwnerComp.GetAIOwner();
	AWolf* Wolf = AttackTask_GetWolf(OwnerComp);
	const UWolfData* Data = Wolf ? Wolf->GetWolfData() : nullptr;
	AActor* Target = AttackTask_GetTarget(OwnerComp);
	
	if (!AIC || !Wolf || !Data || !Target) return EBTNodeResult::Failed;
	
	const float Dist = FVector::Dist2D(Wolf->GetActorLocation(), Target->GetActorLocation());
	Memory->AttackIndex = Wolf->PickAttack(Dist);
	if (Memory->AttackIndex == INDEX_NONE)
	{
		Memory->AttackIndex = Wolf->PickAttack(-1.0f);
	}
	if (Memory->AttackIndex == INDEX_NONE) return EBTNodeResult::Failed;
	
	UWolfPackSubsystem* Pack = Wolf->GetWorld()->GetSubsystem<UWolfPackSubsystem>();
	if (!Pack || !Pack->TryTakeAttackToken(Wolf, Data->MaxSimultaneousAttackers))
	{
		return EBTNodeResult::Failed;
	}

	Memory->bHasToken = true;
	Memory->Timer = Data->ApproachTimeout;

	Wolf->SetRunning(true);
	AIC->MoveToActor(Target,  AttackTask_ApproachRadius(Data->Attacks[Memory->AttackIndex]));
	return EBTNodeResult::InProgress;
}

void UBTTask_WolfAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTWolfAttackMemory* Memory = CastInstanceNodeMemory<FBTWolfAttackMemory>(NodeMemory);
	
	AAIController* AIC = OwnerComp.GetAIOwner();
	AWolf* Wolf = AttackTask_GetWolf(OwnerComp);
	const UWolfData* Data = Wolf ? Wolf->GetWolfData() : nullptr;
	AActor* Target = AttackTask_GetTarget(OwnerComp);
	
	if (!AIC || !Wolf || !Data || !Target || !Data->Attacks.IsValidIndex(Memory->AttackIndex))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	if (Memory->bAttacking)
	{
		if (!Wolf->IsAttacking())
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
		return;
	}
	
	if (Memory->Timer <= 0.0f)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	const float Dist = FVector::Dist2D(Wolf->GetActorLocation(), Target->GetActorLocation());
	const FWolfAttack& Chosen = Data->Attacks[Memory->AttackIndex];
	
	if (Dist < Chosen.MinRange)
	{
		const int32 Better = Wolf->PickAttack(Dist);
		if (Better != INDEX_NONE)
		{
			Memory->AttackIndex = Better;
		}
	}
	
	if (Dist <= Data->Attacks[Memory->AttackIndex].MaxRange)
	{
		AIC->StopMovement();
		if (Wolf->StartAttack(Target, Memory->AttackIndex) > 0.0f)
		{
			Memory->bAttacking = true;
		}
		else
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		}
		return;
	}
	
	if (AIC->GetMoveStatus() == EPathFollowingStatus::Type::Idle)
	{
		AIC->MoveToActor(Target, AttackTask_ApproachRadius(Chosen));
	}
}

void UBTTask_WolfAttack::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	FBTWolfAttackMemory* Memory = CastInstanceNodeMemory<FBTWolfAttackMemory>(NodeMemory);
	AWolf* Wolf = AttackTask_GetWolf(OwnerComp);

	if (Memory->bHasToken && Wolf)
	{
		if (UWolfPackSubsystem* Pack = Wolf->GetWorld()->GetSubsystem<UWolfPackSubsystem>())
		{
			Pack->ReleaseAttackToken(Wolf);
		}
	}
	Memory->bHasToken = false;

	if (!Memory->bAttacking && TaskResult != EBTNodeResult::Succeeded)
	{
		if (AAIController* AIC = OwnerComp.GetAIOwner())
		{
			AIC->StopMovement();
		}
	}
	
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

uint16 UBTTask_WolfAttack::GetInstanceMemorySize() const
{
	return sizeof(FBTWolfAttackMemory);
}
