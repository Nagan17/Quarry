// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animal")
	TObjectPtr<UAnimalData> AnimalData;
};
