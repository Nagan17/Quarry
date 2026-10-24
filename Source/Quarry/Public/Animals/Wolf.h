// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animals/BaseAnimal.h"
#include "Materials/MaterialInterface.h"
#include "Wolf.generated.h"

/**
 * 
 */

class UWolfData;

UCLASS()
class QUARRY_API AWolf : public ABaseAnimal
{
	GENERATED_BODY()
	
public:
	AWolf();
	
	const UWolfData * GetWolfData() const;
	
	//Howl
	bool CanHowl() const;
	float Howl();
	void MarkHowled();
	
	//Attack
	int32 PickAttack(float Distance) const;
	float StartAttack(AActor* Target, int32 AttackIndex);
	bool IsAttacking() const {return bAttacking;}
	
	// Call this from an AnimNotify
	UFUNCTION(BlueprintCallable, Category = "Wolf")
	void AttackHitCheck();
	
	virtual void HandleDeath(UHealthComponent* HealthComponent, AController* Contr) override;
	
	//Appearance
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UPROPERTY(EditAnywhere, Category = "Wolf|Appearance")
	int32 ForcedVariant = -1;
	
private:
	void EndAttack();
	void ApplyRandomVariant();
	
	TWeakObjectPtr<AActor> AttackTarget;
	FTimerHandle AttackHitTimer;
	FTimerHandle AttackEndTimer;
	int32 CurrentAttack = INDEX_NONE;
	bool bAttacking = false;
	bool bHitThisAttack = false;
	float LastHowlTime = -1.0e6f;
	
};
