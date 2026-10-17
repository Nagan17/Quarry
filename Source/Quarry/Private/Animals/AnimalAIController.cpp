// Fill out your copyright notice in the Description page of Project Settings.


#include "Animals/AnimalAIController.h"
#include "Animals/BaseAnimal.h"
#include "Animals/AnimalData.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISense_Damage.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "Navigation/PathFollowingComponent.h"

const FName AAnimalAIController::BB_ThreatActor(TEXT("ThreatActor"));
const FName AAnimalAIController::BB_ThreatLocation(TEXT("ThreatLocation"));
const FName AAnimalAIController::BB_AwarenessState(TEXT("AwarenessState"));
const FName AAnimalAIController::GunshotTag(TEXT("Gunshot"));
const FName AAnimalAIController::BB_HomeLocation(TEXT("HomeLocation"));
const FName AAnimalAIController::BB_MoveTarget(TEXT("MoveTarget"));

AAnimalAIController::AAnimalAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	
	AnimalPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AnimalPerception"));
	SetPerceptionComponent(*AnimalPerception);
	
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 2500.0f;
	SightConfig->LoseSightRadius = 3000.0f;
	SightConfig->PeripheralVisionAngleDegrees = 80.0f;
	SightConfig->SetMaxAge(5.0f);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 3000.0f;
	HearingConfig->SetMaxAge(3.0f);
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	
	DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
	DamageConfig->SetMaxAge(5.0f);
	
	AnimalPerception->ConfigureSense(*SightConfig);
	AnimalPerception->ConfigureSense(*HearingConfig);
	AnimalPerception->ConfigureSense(*DamageConfig);
	AnimalPerception->SetDominantSense(SightConfig->GetSenseImplementation());
}

void AAnimalAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!AnimalData) return;
	
	TImeSinceStimulus += DeltaTime;
	
	UpdateSightAwareness(DeltaTime);
	
	if (TImeSinceStimulus > AnimalData->CalmDownDelay)
	{
		Awareness = FMath::Max(0, Awareness - AnimalData->AwarenessDecayPerSecond * DeltaTime);
	}
	
	UpdateAwarenessState();
	
	if (bDrawDebugAwareness)
	{
		DrawDebugAwareness();
	}
}

void AAnimalAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (UPathFollowingComponent* PathComp = GetPathFollowingComponent())
	{
		PathComp->SetStopMovementOnFinish(false);
	}
	
	Animal = Cast<ABaseAnimal>(InPawn);
	AnimalData = Animal ? Animal->GetAnimalData() : nullptr;
	if (!AnimalData)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: no AnimalData set on %s"), *GetName(), *GetNameSafe(InPawn));
		return;
	}
	
	ConfigureSensesFromData(AnimalData);
	AnimalPerception->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &AAnimalAIController::OnTargetPerceptionUpdate);	
	if (AnimalData->BehaviourTree)
	{
		RunBehaviorTree(AnimalData->BehaviourTree);
		if (UBlackboardComponent* BB = GetBlackboardComponent())
		{
			BB->SetValueAsVector(BB_HomeLocation, InPawn->GetActorLocation());
			BB->SetValueAsEnum(BB_AwarenessState, static_cast<uint8>(AwarenessState));
		}
	}
}

void AAnimalAIController::OnUnPossess()
{
	Super::OnUnPossess();
	Animal = nullptr;
	AnimalData = nullptr;
	VisibleThreat = nullptr;
}

void AAnimalAIController::OnTargetPerceptionUpdate(AActor* Actor, FAIStimulus Stimulus)
{
	if (!AnimalData || !IsThreat(Actor)) return;
	
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			VisibleThreat = Actor;
			SetThreat(Actor, Actor->GetActorLocation());
		}
		else if (VisibleThreat == Actor)
		{
			//Lost Sight: Remember where animal last saw them
			VisibleThreat = nullptr;
			SetThreat(Actor, Stimulus.StimulusLocation);
		}
	}
	else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		if (!Stimulus.WasSuccessfullySensed()) return; // stimulus expired
		
		if (Stimulus.Tag == GunshotTag)
		{
			Awareness = 1.0f;
		}
		else
		{
			const float HearingScale = (Animal->GetPosture() == EAnimalPosture::Sleeping) ? AnimalData->SleepingHearingMultiplier : 1.f;
			AddAwareness(Stimulus.Strength * AnimalData->NoiseAwarenessScale * HearingScale);
		}
		
		SetThreat(Actor, Stimulus.StimulusLocation);
		TImeSinceStimulus = 0;
	}
	else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
	{
		if (!Stimulus.WasSuccessfullySensed()) return;
		
		Awareness = 1.0f;
		SetThreat(Actor, Stimulus.StimulusLocation);
		TImeSinceStimulus = 0;
	}
}

void AAnimalAIController::ConfigureSensesFromData(const UAnimalData* Data)
{
	SightConfig->SightRadius = Data->SightRadius;
	SightConfig->LoseSightRadius = Data->LoseSightRadius;
	SightConfig->PeripheralVisionAngleDegrees = Data->PeripheralVisionAngle;
	HearingConfig->HearingRange = Data->HearingRange;
	
	AnimalPerception->ConfigureSense(*SightConfig);
	AnimalPerception->ConfigureSense(*HearingConfig);
	AnimalPerception->ConfigureSense(*DamageConfig);
	AnimalPerception->RequestStimuliListenerUpdate();
}

bool AAnimalAIController::IsThreat(const AActor* Actor) const
{
	const APawn* ThreatPawn = Cast<APawn>(Actor);
	return ThreatPawn && ThreatPawn->IsPlayerControlled();
}

void AAnimalAIController::UpdateSightAwareness(float DeltaTime)
{
	if (!VisibleThreat) return;
	
	//Closer = notice faster
	const float Dist = FVector::Dist(Animal->GetActorLocation(), VisibleThreat->GetActorLocation());
	const float DistAlpha = FMath::Clamp(Dist / FMath::Max(AnimalData->SightRadius, 1.0f), 0.0f, 1.0f);
	float Rate = AnimalData->SightAwarenessPerSecond * FMath::Lerp(AnimalData->CloseSightMultiplier, AnimalData->FarSightMultiplier, DistAlpha);
	
	// Crouching = harder to spot (relies on ACharacter::Crouch setting bIsCrouched)
	if (const ACharacter* ThreatChar = Cast<ACharacter>(VisibleThreat))
	{
		if (ThreatChar->bIsCrouched)
		{
			Rate *= AnimalData->CrouchSightMultiplier;
		}
	}
	
	switch (Animal->GetPosture())
	{
	case EAnimalPosture::LyingDown:
		
	case EAnimalPosture::Resting:  
		Rate *= AnimalData->RestingSightMultiplier;  
		break;
		
	case EAnimalPosture::Sleeping: 
		Rate *= AnimalData->SleepingSightMultiplier; 
		break;
	default: break;
	}
	
	// Standing still = barely noticeable, and does NOT count as a fresh stimulus,
	// so decay kicks in and the animal can calm down if the player freezes.
	
	const bool bThreatMoving = VisibleThreat->GetVelocity().SizeSquared2D() > 25.0f;
	const bool bTooClose = Dist < AnimalData->StillDetectRadius;
	
	if (bThreatMoving || bTooClose)
	{
		TImeSinceStimulus = 0;
	}
	
	if (!bThreatMoving && !bTooClose)
	{
		Rate *= AnimalData->StillSightMultiplier;
	}
	
	AddAwareness(Rate * DeltaTime);
	SetThreat(VisibleThreat, VisibleThreat->GetActorLocation());
}

void AAnimalAIController::UpdateAwarenessState()
{
	EAwarenessState NewState = AwarenessState;
	switch (AwarenessState)
	{
		case EAwarenessState::Calm:
			if (Awareness >= AnimalData->AlarmedThreshold)
			{
				NewState = EAwarenessState::Alarmed;
			}
			else if (Awareness >= AnimalData->SuspiciousThreshold)
			{
				NewState = EAwarenessState::Suspicious;
			}
		break;
		
		
		case EAwarenessState::Suspicious:
			if (Awareness >= AnimalData->AlarmedThreshold)
			{
				NewState = EAwarenessState::Alarmed;
			}
			else if (Awareness < AnimalData->SuspiciousThreshold * 0.5f)
			{
				NewState = EAwarenessState::Calm;
			}
		break;
		
		case EAwarenessState::Alarmed:
			// After fleeing, drop to Suspicious (stop and look back) before going Calm
			if (Awareness < AnimalData->SuspiciousThreshold)
			{
				NewState = EAwarenessState::Suspicious;
			}
		break;
	}
	
	if (NewState == AwarenessState) return;
	AwarenessState = NewState;
	
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsEnum(BB_AwarenessState, static_cast<uint8>(AwarenessState));
		if (AwarenessState == EAwarenessState::Calm)
		{
			BB->ClearValue(BB_ThreatActor);
		}
	}
}

void AAnimalAIController::AddAwareness(float Amount)
{
	Awareness = FMath::Clamp(Awareness + Amount, 0.0f, 1.0f);
}

void AAnimalAIController::SetThreat(AActor* Actor, const FVector& Location)
{
	LastThreatLocation = Location;
	
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsObject(BB_ThreatActor, Actor);
		BB->SetValueAsVector(BB_ThreatLocation, Location);
	}
}

void AAnimalAIController::DrawDebugAwareness() const
{
#if ENABLE_DRAW_DEBUG
	if (!Animal) return;
	
	FColor Color = FColor::Green;
	if (AwarenessState == EAwarenessState::Suspicious) Color = FColor::Yellow;
	if (AwarenessState == EAwarenessState::Alarmed)    Color = FColor::Red;
	
	const FString Text = FString::Printf(TEXT("%s  %.2f"),
		*UEnum::GetDisplayValueAsText(AwarenessState).ToString(), Awareness);
	
	// Location is relative to the animal when a base actor is passed
	DrawDebugString(GetWorld(), FVector(0.f, 0.f, 150.f), Text, Animal, Color, 0.f, true);
 
	if (AwarenessState != EAwarenessState::Calm)
	{
		DrawDebugLine(GetWorld(), Animal->GetActorLocation(), LastThreatLocation, Color, false, 0.f, 0, 1.5f);
	}
	
#endif
}
