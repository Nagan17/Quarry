// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Weapon.h"

#include "Characters/PlayerCharacter.h"
#include "Weapons/Projectile.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	// The weapon shouldn't push the capsule or block the camera
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

bool AWeapon::TryFire(const FVector& AimPoint, bool bAiming, TOptional<FVector> OriginOverride)
{
	if (State != EWeaponState::Ready) return false;
	
	if (!bRoundChambered)
	{
		StartCycleOrReload();
		return false;
	}
	
	const FVector Origin = OriginOverride.IsSet()
	? OriginOverride.GetValue()
	: WeaponMesh->GetSocketLocation(MuzzleSocket);
	
	SpawnProjectile(Origin, AimPoint, bAiming ? ADSSpreadDeg : HipSpreadDeg);
	bRoundChambered = false;
	BroadcastAmmo();
	
	SetState(EWeaponState::Recovering);
	GetWorldTimerManager().SetTimer(ActionTimer, this, &AWeapon::StartCycleOrReload, PostShotDelay, false);
	return true;
}

bool AWeapon::TryReload()
{
	if (State != EWeaponState::Ready || ReserveAmmo <= 0) return false;
	if (MagAmmo >= MagCapacity && bRoundChambered) return false;
	StartFullReload();
	return true;
}

void AWeapon::CancelAction()
{
	GetWorldTimerManager().ClearTimer(ActionTimer);
	if (State != EWeaponState::Ready)
	{
		SetState(EWeaponState::Ready);
	}
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	MagAmmo = MagCapacity;
	ReserveAmmo = StartingReserve;
	bRoundChambered = true;
	BroadcastAmmo();
}

void AWeapon::SpawnProjectile(const FVector& Origin, const FVector& AimPoint, float SpreadDeg)
{
	if (!ProjectileClass || !WeaponMesh->DoesSocketExist(MuzzleSocket)) return;
	
	FVector Dir = (AimPoint - Origin).GetSafeNormal();
	if (SpreadDeg > 0.0f)
	{
		Dir = FMath::VRandCone(Dir, FMath::DegreesToRadians(SpreadDeg));
	}
	
	const FTransform SpawnTM(Dir.Rotation(), Origin);
	
	AProjectile* Bullet = GetWorld()->SpawnActorDeferred<AProjectile>(
		ProjectileClass, SpawnTM, this, GetInstigator(),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	
	if (Bullet)
	{
		Bullet->InitBallistics(MuzzleVelocity, BulletGravityScale);
		Bullet->FinishSpawning(SpawnTM);
	}
}

void AWeapon::StartCycleOrReload()
{
	if (MagAmmo > 0)
	{
		SetState(EWeaponState::Cycling);
		GetWorldTimerManager().SetTimer(ActionTimer, this, &AWeapon::FinishCycle, CycleTime, false);
	}
	else if (ReserveAmmo > 0)
	{
		StartFullReload();
	}
	else
	{
		SetState(EWeaponState::Ready);
	}
}

void AWeapon::StartFullReload()
{
	SetState(EWeaponState::Reloading);
	GetWorldTimerManager().SetTimer(ActionTimer, this, &AWeapon::FinishReload, ReloadTime, false);
}

void AWeapon::FinishCycle()
{
	--MagAmmo;
	bRoundChambered = true;
	SetState(EWeaponState::Ready);
	BroadcastAmmo();
}

void AWeapon::FinishReload()
{
	const int32 Take = FMath::Min(MagCapacity - MagAmmo, ReserveAmmo);
	MagAmmo += Take;
	ReserveAmmo -= Take;
	
	if (!bRoundChambered && MagAmmo > 0)
	{
		--MagAmmo;
		bRoundChambered = true;
	}
	SetState(EWeaponState::Ready);
	BroadcastAmmo();
}

void AWeapon::SetState(EWeaponState NewState)
{
	State = NewState;
	OnStateChanged.Broadcast(NewState);
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

