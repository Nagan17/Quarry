// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class UHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthChanged, UHealthComponent*, HealthComp, float, NewHealth, float, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeath, UHealthComponent*, HealthComp, AController*, Killer);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class QUARRY_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

	UPROPERTY(BlueprintAssignable) FOnHealthChanged OnHealthChanged;
	UPROPERTY(BlueprintAssignable) FOnDeath OnDeath;
	
	UFUNCTION(BlueprintPure) 
	float GetHealth() const
	{
		return CurrentHealth;
	}
	
	UFUNCTION(BlueprintPure) 
	float GetHealthPercent() const
	{
		return CurrentHealth / MaxHealth;
	}
	
	UFUNCTION(BlueprintPure) 
	bool IsDead() const
	{
		return CurrentHealth <= 0.0f;
	}
	
	UFUNCTION(BlueprintCallable) 
	void Heal(float Amount);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, Category="Health", meta=(ClampMin="1"))
	float MaxHealth = 100.f;

	UPROPERTY(VisibleInstanceOnly, Category="Health")
	float CurrentHealth = 0.f;

	UFUNCTION()
	void HandleAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
