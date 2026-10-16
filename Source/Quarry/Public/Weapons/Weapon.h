// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class AProjectile;
class UAnimMontage;

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	Ready, 
	Recovering, 
	Cycling, 
	Reloading
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAmmoChanged, bool, bChambered, int32, MagAmmo, int32, ReserveAmmo);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnWeaponStateChanged, EWeaponState);

UCLASS()
class QUARRY_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();
	
	UStaticMeshComponent* GetWeaponMesh() const {return WeaponMesh; }
	
	UPROPERTY(BlueprintAssignable)
	FOnAmmoChanged OnAmmoChanged;
	
	FOnWeaponStateChanged OnStateChanged;
	
	bool TryFire(const FVector& AimPoint, bool bAiming, TOptional<FVector> OriginOverride = {});
	bool TryReload();
	void CancelAction();
	
	EWeaponState GetState() const { return State; }
	UAnimMontage* GetCycleMontage()  const { return CycleMontage; }
	UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
	float GetCycleTime()  const { return CycleTime; }
	float GetReloadTime() const { return ReloadTime; }
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon") 
	TSubclassOf<AProjectile> ProjectileClass;
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon") 
	FName MuzzleSocket = TEXT("MuzzleSocket");

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Ammo", meta=(ClampMin="1"))
	int32 MagCapacity = 5;
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Ammo") 
	int32 StartingReserve = 20;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Timing") 
	float PostShotDelay = 0.35f;
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Timing") 
	float CycleTime     = 0.9f;
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Timing") 
	float ReloadTime    = 2.5f;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Accuracy") 
	float HipSpreadDeg = 2.5f;
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Accuracy") 
	float ADSSpreadDeg = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Animation") 
	TObjectPtr<UAnimMontage> CycleMontage;
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Animation") 
	TObjectPtr<UAnimMontage> ReloadMontage;

	UPROPERTY(VisibleInstanceOnly, Category="Weapon|Ammo") 
	bool  bRoundChambered = true;
	
	UPROPERTY(VisibleInstanceOnly, Category="Weapon|Ammo") 
	int32 MagAmmo = 0;
	
	UPROPERTY(VisibleInstanceOnly, Category="Weapon|Ammo") 
	int32 ReserveAmmo = 0;
	
	EWeaponState State = EWeaponState::Ready;
	FTimerHandle ActionTimer;
	
	//Bullet
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Ballistics", meta = (ClampMin = "100"))
	float MuzzleVelocity = 40000.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Ballistics", meta = (ClampMin = "0"))
	float BulletGravityScale = 1.0f;

	void SpawnProjectile(const FVector& Origin, const FVector& AimPoint, float SpreadDeg);
	void StartCycleOrReload();
	void StartFullReload();
	void FinishCycle();
	void FinishReload();
	void SetState(EWeaponState NewState);
	void BroadcastAmmo() { OnAmmoChanged.Broadcast(bRoundChambered, MagAmmo, ReserveAmmo); }

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
