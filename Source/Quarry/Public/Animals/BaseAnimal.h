// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimalData.h"
#include "GameFramework/Character.h"
#include "BaseAnimal.generated.h"

class UAnimalData;

UCLASS(Abstract)
class QUARRY_API ABaseAnimal : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseAnimal();
	
	UFUNCTION(BlueprintPure, Category = "Animal")
	UAnimalData* GetAnimalData() const { return AnimalData; }
	
	UFUNCTION(BlueprintCallable, Category = "Animal")
	void SetRunning(bool bRun);
	
	//Grazing
	void SetGrazing(bool bGrazing) { bIsGrazing = bGrazing; }
	bool IsGrazing() const { return bIsGrazing; }
	
	//Posture
	UFUNCTION(BlueprintPure, Category = "Animal|Posture")
	EAnimalPosture GetPosture() const { return Posture; }
	
	bool IsStanding() const { return Posture == EAnimalPosture::Standing; }
	void StartLyingDown();
	void FallAsleep();
	void StartGettingUp();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animal")
	TObjectPtr<UAnimalData> AnimalData;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Animal")
	bool bIsGrazing = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Animal|Posture")
	EAnimalPosture Posture = EAnimalPosture::Standing;
	
private:
	void SetPosture(EAnimalPosture NewPosture);
	
	float PlayPostureMontage(UAnimMontage* Montage);
	
	FTimerHandle PostureTimer;

};
