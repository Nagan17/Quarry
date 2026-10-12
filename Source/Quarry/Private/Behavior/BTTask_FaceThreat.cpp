// Fill out your copyright notice in the Description page of Project Settings.


#include "Behavior/BTTask_FaceThreat.h"

#include "AIController.h"
#include "Animals/AnimalAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_FaceThreat::UBTTask_FaceThreat()
{
	NodeName = TEXT("Face Threat");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_FaceThreat::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC || !AIC->GetPawn()) return EBTNodeResult::Failed;
 
	AIC->StopMovement();
	return EBTNodeResult::InProgress;
}

void UBTTask_FaceThreat::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	const AAIController* AIC = OwnerComp.GetAIOwner();
	APawn* AnimalPawn = AIC ? AIC->GetPawn() : nullptr;
	const UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!AnimalPawn || !BB)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	const FVector ToThreat = BB->GetValueAsVector(AAnimalAIController::BB_ThreatLocation) - AnimalPawn->GetActorLocation();
	if (ToThreat.SizeSquared2D() < 1.0f)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
	
	const float DesiredYaw = ToThreat.Rotation().Yaw;
	FRotator Rotation = AnimalPawn->GetActorRotation();
	Rotation.Yaw = FMath::FixedTurn(Rotation.Yaw, DesiredYaw, TurnSpeed * DeltaSeconds);
	AnimalPawn->SetActorRotation(Rotation);
	
	if (FMath::Abs(FMath::FindDeltaAngleDegrees(Rotation.Yaw, DesiredYaw)) <= AcceptableAngle)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

FString UBTTask_FaceThreat::GetStaticDescription() const
{
	return FString::Printf(TEXT("Turn %.0f deg/s"), TurnSpeed);
}
