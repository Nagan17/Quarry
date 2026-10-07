// Fill out your copyright notice in the Description page of Project Settings.


//#include "Characters/BaseCharacter.h"
#include "BaseCharacter.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Weapons/Weapon.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
	
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
	if (bNewAiming && !bIsArmed)
	{
		return;
	}
	
	bIsAiming = bNewAiming;
	UCharacterMovementComponent* Move = GetCharacterMovement();
	
	Move->bOrientRotationToMovement = !bIsAiming;
	Move->bUseControllerDesiredRotation = bIsAiming;
	
	UpdateMovementSpeed();
}

void ABaseCharacter::SetArmed(bool bNewArmed)
{
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
	
	if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
	{
		if (!Anim->Montage_IsPlaying(FireMontage))
		{
			Anim->Montage_Play(FireMontage);
		}
	}
	
	// TODO: spawn projectile / line trace, ammo, recoil, etc.
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
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

