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
	virtual void SetArmed(bool bNewArmed);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ToggleArmed();
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void Fire();
	
	void ExecuteFire();
	
	//Weapon
	UFUNCTION(BlueprintCallable, Category="Combat")
	void Reload();
	
	virtual TOptional<FVector> GetShotOriginOverride() const { return {}; }
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void OnAmmoChanged(bool bChambered, int32 MagAmmo, int32 ReserveAmmo);
	
	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const { return bIsDead; }
	
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
	UPROPERTY(VisibleAnywhere, Category = "Health")
	UHealthComponent* HealthComp;
	
	UFUNCTION()
	virtual void HandleDeath(UHealthComponent* HealthComponent, AController* Contr);
	
	UFUNCTION()
	virtual void HandleHealthChanged(UHealthComponent* HealthComponent, float NewHealth, float Delta);
	
	virtual void OnHurt(float DamageTaken) {}
	
	UPROPERTY(EditDefaultsOnly, Category="Health")
	TMap<FName, float> BoneDamageMultipliers;
	
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
	// Hurt
	UPROPERTY(EditDefaultsOnly, Category = "Health|Hurt")
	TArray<TObjectPtr<UAnimMontage>> HurtMontages;

	UPROPERTY(EditDefaultsOnly, Category = "Health|Hurt", meta = (ClampMin = "0"))
	float HurtCooldown = 0.6f;

	UPROPERTY(EditDefaultsOnly, Category = "Health|Hurt")
	bool bHurtBreaksAim = true;

	// Death
	UPROPERTY(EditDefaultsOnly, Category = "Health|Death")
	TObjectPtr<UAnimMontage> DeathMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = "Health|Death")
	bool bRagdollOnDeath = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Health")
	bool bIsDead = false;

	void StartRagdoll();

	float LastHurtTime = -100.f;
	FTimerHandle DeathTimer;
	
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
	
	//Hip-Fire
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Animaiton")
	FName FireNotifyName = TEXT("Fire");

	bool bFirePending = false;
	
	UFUNCTION()
	void OnFireMontageNotify(FName NotifyName, const FBranchingPointNotifyPayload& Payload);

	void OnFireMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
