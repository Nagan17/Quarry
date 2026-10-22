// Fill out your copyright notice in the Description page of Project Settings.


#include "Animals/WolfPackSubsystem.h"

#include "Animals/AnimalAIController.h"
#include "Animals/Wolf.h"
#include "Components/HealthComponent.h"

void UWolfPackSubsystem::RegisterWolf(AWolf* Wolf)
{
	if (Wolf)
	{
		Wolves.AddUnique(TWeakObjectPtr<AWolf>(Wolf));
	}
}

void UWolfPackSubsystem::UnregisterWolf(AWolf* Wolf)
{
	Wolves.Remove(TWeakObjectPtr<AWolf>(Wolf));
	Attackers.Remove(TWeakObjectPtr<AWolf>(Wolf));
}

int32 UWolfPackSubsystem::CallPack(AWolf* Caller, AActor* Target, const FVector& TargetLocation, float Radius,
	int32 MaxWolves)
{
	CleanUp();
	if (!Caller || !Target || MaxWolves <= 0) return 0;
	
	const FVector Origin = Caller->GetActorLocation();
	const float RadiusSq = FMath::Square(Radius);
	int32 Remaining = MaxWolves;
	
	TArray<AWolf*> Candidates;
	for (const TWeakObjectPtr<AWolf>& CandidateWolf : Wolves)
	{
		AWolf* Wolf = CandidateWolf.Get();
		if (!Wolf || Wolf == Caller || Wolf->HealthComp->IsDead()) continue;
		if (FVector::DistSquared(Origin, Wolf->GetActorLocation()) > RadiusSq) continue;
		
		const AAnimalAIController* AIC = Cast<AAnimalAIController>(Wolf->GetController());
		if (!AIC) continue;
		
		if (AIC->GetAwarenessState() == EAwarenessState::Alarmed)
		{
			Wolf->MarkHowled();
			--Remaining;
			continue;
		}
		
		Candidates.Add(Wolf);
	}
	
	Candidates.Sort([&Origin](const AWolf& A, const AWolf& B)
	{
		return FVector::DistSquared(Origin, A.GetActorLocation()) < FVector::DistSquared(Origin, B.GetActorLocation());
	});
	
	const int32 Count = FMath::Clamp(Remaining, 0, Candidates.Num());
	for (int32 i = 0; i < Count; ++i)
	{
		Candidates[i]->MarkHowled();
		if (AAnimalAIController* AIC = Cast<AAnimalAIController>(Candidates[i]->GetController()))
		{
			AIC->AlertToThreat(Target, TargetLocation);
		}
	}
	return Count;
}

bool UWolfPackSubsystem::TryTakeAttackToken(AWolf* Wolf, int32 MaxAttackers)
{
	CleanUp();
	const TWeakObjectPtr<AWolf> Weak(Wolf);
	if (Attackers.Contains(Weak)) return true;
	if (Attackers.Num() >= MaxAttackers) return false;
	
	Attackers.Add(Weak);
	return true;
}

void UWolfPackSubsystem::ReleaseAttackToken(AWolf* Wolf)
{
	Attackers.Remove(TWeakObjectPtr<AWolf>(Wolf));
}

void UWolfPackSubsystem::CleanUp()
{
	Wolves.RemoveAll([](const TWeakObjectPtr<AWolf>& W) { return !W.IsValid(); });
	Attackers.RemoveAll([](const TWeakObjectPtr<AWolf>& W) { return !W.IsValid() || W->HealthComp->IsDead(); });
}
