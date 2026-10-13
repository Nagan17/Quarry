// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Weapon.h"
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

bool AWeapon::TryFire(const FVector& AimPoint, bool bAiming)
{
	if (State != EWeaponState::Ready) return false;
	
	if (!bRoundChambered)
	{
		StartCycleOrReload();
		return false;
	}
	
	SpawnProjectile(AimPoint, bAiming ? ADSSpreadDeg : HipSpreadDeg);
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

void AWeapon::SpawnProjectile(const FVector& AimPoint, float SpreadDeg)
{
	if (!ProjectileClass || !WeaponMesh->DoesSocketExist(MuzzleSocket)) return;
	
	const FVector MuzzleLocation = WeaponMesh->GetSocketLocation(MuzzleSocket);
	FVector Dir = (AimPoint - MuzzleLocation).GetSafeNormal();
	if (SpreadDeg > 0.0f)
	{
		Dir = FMath::VRandCone(Dir, FMath::DegreesToRadians(SpreadDeg));
	}
	
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = GetInstigator();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GetWorld()->SpawnActor<AProjectile>(ProjectileClass, MuzzleLocation, Dir.Rotation(), Params);
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

