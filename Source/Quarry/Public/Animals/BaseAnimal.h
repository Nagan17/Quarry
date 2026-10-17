// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimalData.h"
#include "GameFramework/Character.h"
#include "BaseAnimal.generated.h"

class UHealthComponent;
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
	
	//Health
	UPROPERTY(VisibleAnywhere, Category = "Health")
	UHealthComponent* HealthComp;

	
	UFUNCTION()
	virtual void HandleDeath(UHealthComponent* HealthComponent, AController* Contr);
	
	UPROPERTY(EditDefaultsOnly, Category="Health")
	TMap<FName, float> BoneDamageMultipliers;
	
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animal")
	TObjectPtr<UAnimalData> AnimalData;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Animal")
	bool bIsGrazing = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Animal|Posture")
	EAnimalPosture Posture = EAnimalPosture::Standing;
	
	//Ragdoll
	UPROPERTY(EditDefaultsOnly, Category="Health|Death")
	float DeathMomentumScale = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, Category="Health|Death")
	float BulletImpulse = 3000.0f;
	
	UPROPERTY(EditDefaultsOnly, Category="Health|Death")
	float RagdollLinearDamping = 0.05f;
	
	UPROPERTY(EditDefaultsOnly, Category="Health|Death")
	float StumbleTime = 0.6f;
	
	UPROPERTY(EditDefaultsOnly, Category="Health|Death")
	float StumbleDeceleration = 500.0f;
	
	//Health
	UPROPERTY(EditDefaultsOnly, Category="Health")
	float HurtCooldown = 0.5f;
	
	UFUNCTION()
	void HandleHealthChanged(UHealthComponent* HealthComponent, float NewHealth, float Delta);
	
private:
	void SetPosture(EAnimalPosture NewPosture);
	float PlayPostureMontage(UAnimMontage* Montage);
	
	void StartRagdoll();
	FTimerHandle DeathTimer;
	bool bIsDying = false;
	
	FName LastHitBone = NAME_None;
	FVector LastHitDirection = FVector::ZeroVector;
	FVector LastHitLocation = FVector::ZeroVector;
	
	float LastHurtTime = -100.f;
	
	FTimerHandle PostureTimer;
};
