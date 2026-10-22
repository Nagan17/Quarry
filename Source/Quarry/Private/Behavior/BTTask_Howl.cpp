// Fill out your copyright notice in the Description page of Project Settings.


#include "Behavior/BTTask_Howl.h"
#include "AIController.h"
#include "Animals/AnimalAIController.h"
#include "Animals/Wolf.h"
#include "Animals/WolfPackSubsystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Datas/WolfData.h"

UBTTask_Howl::UBTTask_Howl()
{
	NodeName = TEXT("Howl");
	bNotifyTick = true;
	bNotifyTaskFinished = true;
}

static AWolf* HowlTask_GetWolf(const UBehaviorTreeComponent& OwnerComp)
{
	const AAIController* AIC = OwnerComp.GetAIOwner();
	return AIC ? Cast<AWolf>(AIC->GetPawn()) : nullptr;
}

EBTNodeResult::Type UBTTask_Howl::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTHowlMemory* Memory = CastInstanceNodeMemory<FBTHowlMemory>(NodeMemory);
	Memory->TimeLeft = 0.f;
	Memory->bTurning = false;

	AWolf* Wolf = HowlTask_GetWolf(OwnerComp);
	if (!Wolf || !Wolf->GetWolfData()) return EBTNodeResult::Failed;

	if (!Wolf->CanHowl()) return EBTNodeResult::Succeeded;

	if (AAIController* AIC = OwnerComp.GetAIOwner())
	{
		AIC->StopMovement();
	}

	Memory->bTurning = true;
	return EBTNodeResult::InProgress;
}

void UBTTask_Howl::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTHowlMemory* Memory = CastInstanceNodeMemory<FBTHowlMemory>(NodeMemory);
	AWolf* Wolf = HowlTask_GetWolf(OwnerComp);
	const UWolfData* Data = Wolf ? Wolf->GetWolfData() : nullptr;
	const UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!Wolf || !Data || !BB)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	if (Memory->bTurning)
	{
		const FVector ThreatLoc = BB->GetValueAsVector(AAnimalAIController::BB_ThreatLocation);
		const FVector ToThreat = ThreatLoc - Wolf->GetActorLocation();

		bool bFacing = ToThreat.SizeSquared2D() < 1.0f;
		if (!bFacing)
		{
			const float DesiredYaw = ToThreat.Rotation().Yaw;
			FRotator Rotation = Wolf->GetActorRotation();
			Rotation.Yaw = FMath::FixedTurn(Rotation.Yaw, DesiredYaw, Data->HowlTurnSpeed * DeltaSeconds);
			Wolf->SetActorRotation(Rotation);
			bFacing = FMath::Abs(FMath::FindDeltaAngleDegrees(Rotation.Yaw, DesiredYaw)) <= 10.0f;
		}
		if (!bFacing) return;

		Memory->bTurning = false;

		if (!Wolf->CanHowl())
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			return;
		}

		Memory->TimeLeft = Wolf->Howl();

		AActor* Target = Cast<AActor>(BB->GetValueAsObject(AAnimalAIController::BB_ThreatActor));
		if (UWolfPackSubsystem* Pack = Wolf->GetWorld()->GetSubsystem<UWolfPackSubsystem>())
		{
			Pack->CallPack(Wolf, Target, ThreatLoc, Data->PackCallRadius, Data->MaxPackCalls);
		}

		if (Memory->TimeLeft <= 0.0f)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
		return;
	}
	
	Memory->TimeLeft -= DeltaSeconds;
	if (Memory->TimeLeft <= 0.0f)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
	
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
}

void UBTTask_Howl::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	if (TaskResult == EBTNodeResult::Aborted)
	{
		AWolf* Wolf = HowlTask_GetWolf(OwnerComp);
		const UWolfData* Data = Wolf ? Wolf->GetWolfData() : nullptr;
		if (Data && Data->HowlMontage)
		{
			Wolf->StopAnimMontage(Data->HowlMontage);
		}
	}
	
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

uint16 UBTTask_Howl::GetInstanceMemorySize() const
{
	return sizeof(FBTHowlMemory);
}
