// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "AnimalData.h"
#include "AnimalAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;
class ABaseAnimal;

/**
 * 
 */
UCLASS()
class QUARRY_API AAnimalAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AAnimalAIController();
	
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintPure, Category = "Awareness")
	float GetAwareness() const { return Awareness; }
	
	UFUNCTION(BlueprintPure, category = "Awareness")
	EAwarenessState GetAwarenessState() const { return AwarenessState; }
	
	static const FName BB_ThreatActor;
	static const FName BB_ThreatLocation;
	static const FName BB_AwarenessState;
	static const FName BB_HomeLocation;
	static const FName BB_MoveTarget;
	
	static const FName GunshotTag;
	
protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	
	UFUNCTION()
	void OnTargetPerceptionUpdate(AActor* Actor, FAIStimulus Stimulus);
	
	UPROPERTY(VisibleAnywhere, Category = "AI")
	TObjectPtr<UAIPerceptionComponent> AnimalPerception;
	
	UPROPERTY(VisibleAnywhere, Category = "AI")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;
	
	UPROPERTY(VisibleAnywhere, Category = "AI")
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;
	
	UPROPERTY(VisibleAnywhere, Category = "AI")
	TObjectPtr<UAISenseConfig_Damage> DamageConfig;
	
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawDebugAwareness = false;
	
private:
	void ConfigureSensesFromData(const UAnimalData* Data);
	bool IsThreat(const AActor* Actor) const;
	void UpdateSightAwareness(float DeltaTime);
	void UpdateAwarenessState();
	void AddAwareness(float Amount);
	void SetThreat(AActor* Actor, const FVector& Location);
	void DrawDebugAwareness() const;
	
	UPROPERTY()
	TObjectPtr<const UAnimalData> AnimalData;
	
	UPROPERTY()
	TObjectPtr<ABaseAnimal> Animal;
	
	UPROPERTY()
	TObjectPtr<AActor> VisibleThreat;
	
	float Awareness = 0.0f;
	float TImeSinceStimulus = 0.0f;
	EAwarenessState AwarenessState = EAwarenessState::Calm;
	FVector LastThreatLocation = FVector::ZeroVector;
};
