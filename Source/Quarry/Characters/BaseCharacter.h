// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Weapons/Weapon.h"
#include "BaseCharacter.generated.h"

class UAnimMontage;
class UHealthComponent;

UCLASS(Abstract)
class QUARRY_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();
	
	
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetSprinting(bool bNewSprinting);
	
	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsSprinting() const { return bIsSprinting; }
	
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetCrouching(bool bNewCrouching);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void ToggleCrouch();

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsCrouching() const { return bIsCrouched; }
	
	//Read by the AnimInstance
	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAiming() const { return bIsAiming; }
	
	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsArmed() const { return bIsArmed ; }
	
	UFUNCTION(BlueprintPure, Category = "Combat")
	AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }
	
	
	//Actions
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void SetAiming(bool bNewAiming);
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetArmed(bool bNewArmed);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ToggleArmed();
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void Fire();
	
	//Weapon
	UFUNCTION(BlueprintCallable, Category="Combat")
	void Reload();

	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	void UpdateMovementSpeed();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Animaiton")
	TObjectPtr<UAnimMontage> FireMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Animaiton")
	TObjectPtr<UAnimMontage> ADSMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float SprintSpeed = 500.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement")
	bool bIsSprinting = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed = 230.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float AimWalkSpeed = 250.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float CrouchSpeed = 150.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float CrouchedHalfHeight = 60.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	bool bIsAiming = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	bool bIsArmed = false;
	
	//Health
	UPROPERTY(BlueprintReadOnly, Category = "Health")
	UHealthComponent* HealthComp;
	
	UFUNCTION()
	virtual void HandleDeath(UHealthComponent* HealthComponent, AController* Contr);
	
	UPROPERTY(EditDefaultsOnly, Category="Health")
	TMap<FName, float> BoneDamageMultipliers;
	
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
	//Weapon
	virtual void Destroyed() override;

	void SpawnDefaultWeapon();
	void AttachWeaponToSocket(FName SocketName);

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Weapon")
	TSubclassOf<AWeapon> DefaultWeaponClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|Weapon")
	TObjectPtr<AWeapon> EquippedWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Weapon")
	FName BackWeaponSocket = TEXT("BackWeaponSocket");

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Weapon")
	FName HandWeaponSocket = TEXT("RightHandWeaponSocket");
	
	virtual FVector GetAimPoint() const;
	
	void HandleWeaponStateChanged(EWeaponState NewState);
	void FaceAimDirection();
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
