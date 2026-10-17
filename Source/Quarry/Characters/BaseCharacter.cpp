// Fill out your copyright notice in the Description page of Project Settings.


//#include "Characters/BaseCharacter.h"
#include "BaseCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Weapons/Weapon.h"
#include "Components/HealthComponent.h"
#include "Engine/DamageEvents.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
	
	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->bOrientRotationToMovement = true;
	Move->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	Move->JumpZVelocity = 700.0f;
	Move->AirControl = 0.35f;
	Move->MaxWalkSpeed = WalkSpeed;
	Move->MinAnalogWalkSpeed = 20.0f;
	Move->BrakingDecelerationWalking = 2000.0f;
	Move->BrakingDecelerationFalling = 1500.0f;
}

void ABaseCharacter::SetSprinting(bool bNewSprinting)
{
	if (bIsAiming)
	{
		return;
	}
	
	if (bNewSprinting && GetCharacterMovement()->bWantsToCrouch)
	{
		UnCrouch();
	}
	
	bIsSprinting = bNewSprinting;
	UpdateMovementSpeed();
}

void ABaseCharacter::SetCrouching(bool bNewCrouching)
{
	if (bNewCrouching)
	{
		if (bIsSprinting)
		{
			SetSprinting(false);
		}
		Crouch();
	}
	else
	{
		UnCrouch();
	}
}

void ABaseCharacter::ToggleCrouch()
{
	SetCrouching(!GetCharacterMovement()->bWantsToCrouch);
}

void ABaseCharacter::SetAiming(bool bNewAiming)
{
	if (bNewAiming && (!bIsArmed || GetCharacterMovement()->IsFalling())) return;
	if (bIsAiming == bNewAiming) return;

	bIsAiming = bNewAiming;
	if (bIsAiming) SetSprinting(false);
	
	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->bOrientRotationToMovement     = !bIsAiming;
	Move->bUseControllerDesiredRotation =  bIsAiming;
	if (bIsAiming) Move->RotationRate = FRotator(0.f, 720.f, 0.f);
	UpdateMovementSpeed();

	if (UAnimInstance* Anim = GetMesh()->GetAnimInstance(); Anim && ADSMontage)
	{
		if (bIsAiming) Anim->Montage_Play(ADSMontage);
		else           Anim->Montage_Stop(0.2f, ADSMontage);
	}
}

void ABaseCharacter::SetArmed(bool bNewArmed)
{
	if (!bIsArmed)
	{
		EquippedWeapon->CancelAction();
		GetMesh()->GetAnimInstance()->StopAllMontages(0.2f);
	}
	
	if (!EquippedWeapon || bIsArmed == bNewArmed)
	{
		return;
	}

	bIsArmed = bNewArmed;
	AttachWeaponToSocket(bIsArmed ? HandWeaponSocket : BackWeaponSocket);

	// Can't stay aimed with the rifle on your back
	if (!bIsArmed && bIsAiming)
	{
		SetAiming(false);
	}
}

void ABaseCharacter::ToggleArmed()
{
	SetArmed(!bIsArmed);
}

void ABaseCharacter::Fire()
{
	if (!bIsArmed || !FireMontage)
	{
		return;
	}
	
	if (!bIsAiming)
	{
		FaceAimDirection();
	}
	
	if (EquippedWeapon->TryFire(GetAimPoint(), bIsAiming, GetShotOriginOverride()) && FireMontage)
	{
		if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
		{
			Anim->Montage_Play(FireMontage);
		}
	}
}

void ABaseCharacter::Reload()
{
	if (bIsArmed && EquippedWeapon)
	{
		EquippedWeapon->TryReload();
	}
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	HealthComp->OnDeath.AddUniqueDynamic(this, &ABaseCharacter::HandleDeath);
	
	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->GetNavAgentPropertiesRef().bCanCrouch = true;
	Move->SetCrouchedHalfHeight(CrouchedHalfHeight);
	Move->MaxWalkSpeedCrouched = CrouchSpeed;
	
	UpdateMovementSpeed();
	SpawnDefaultWeapon();
}

void ABaseCharacter::UpdateMovementSpeed()
{
	float Speed = WalkSpeed;

	if (bIsAiming)
	{
		Speed = AimWalkSpeed;
	}
	else if (bIsSprinting)
	{
		Speed = SprintSpeed;
	}

	GetCharacterMovement()->MaxWalkSpeed = Speed;
}

void ABaseCharacter::HandleDeath(UHealthComponent* HealthComponent, AController* Contr)
{
	
}

float ABaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const auto& PointEvent = static_cast<const FPointDamageEvent&>(DamageEvent);
		if (const float* Mult = BoneDamageMultipliers.Find(PointEvent.HitInfo.BoneName))
		{
			DamageAmount = *Mult;
		}
	}
	
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void ABaseCharacter::Destroyed()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}
	Super::Destroyed();
}

void ABaseCharacter::SpawnDefaultWeapon()
{
	if (!DefaultWeaponClass || !GetWorld())
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	EquippedWeapon = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass, GetActorTransform(), Params);
	EquippedWeapon->OnStateChanged.AddUObject(this, &ABaseCharacter::HandleWeaponStateChanged);
	
	bIsArmed = false;
	AttachWeaponToSocket(BackWeaponSocket);
}

void ABaseCharacter::AttachWeaponToSocket(FName SocketName)
{
	if (!EquippedWeapon)
	{
		return;
	}

	EquippedWeapon->AttachToComponent(GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
}

FVector ABaseCharacter::GetAimPoint() const
{
	FVector Loc; 
	FRotator Rot;
	GetActorEyesViewPoint(Loc, Rot);
	return Loc + Rot.Vector() * 10000.f;
}

void ABaseCharacter::HandleWeaponStateChanged(EWeaponState NewState)
{
	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	if (!Anim || !EquippedWeapon) return;
	
	auto PlayTimed = [Anim](UAnimMontage* M, float Duration)
	{
		if (M && Duration > 0)
		{
			Anim->Montage_Play(M, M->GetPlayLength() / Duration);
		}
	};

	switch (NewState)
	{
	case EWeaponState::Cycling:
		PlayTimed(EquippedWeapon->GetCycleMontage(), EquippedWeapon->GetCycleTime());
		break;
		
	case EWeaponState::Reloading:
		if (bIsAiming)
		{
			SetAiming(false);
		}
		PlayTimed(EquippedWeapon->GetReloadMontage(), EquippedWeapon->GetReloadTime());
		break;
		
	default:
		break;
	}
}

void ABaseCharacter::FaceAimDirection()
{
	SetActorRotation(FRotator(0, GetControlRotation().Yaw, 0));
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

