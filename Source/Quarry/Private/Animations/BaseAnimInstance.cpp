// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/BaseAnimInstance.h"
#include "KismetAnimationLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Quarry/Characters/BaseCharacter.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Weapons/Weapon.h"

void UBaseAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	Character = Cast<ABaseCharacter>(TryGetPawnOwner());
	if (Character)
	{
		MovementComponent = Character->GetCharacterMovement();
	}
}

void UBaseAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (!Character)
	{
		Character = Cast<ABaseCharacter>(TryGetPawnOwner());
		if (Character)
		{
			MovementComponent = Character->GetCharacterMovement();
		}
	}
	
	if (!Character || !MovementComponent)
	{
		return;
	}
	
	Velocity = MovementComponent->Velocity;
	GroundSpeed = Velocity.Size2D();
	
	const bool bHasAcceleration = !MovementComponent->GetCurrentAcceleration().IsNearlyZero();
	bShouldMove = GroundSpeed > 3.0f && bHasAcceleration;
	bIsFalling = MovementComponent->IsFalling();
	
	Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, Character->GetActorRotation());
	
	bIsAiming = Character->IsAiming();
	bIsArmed = Character->IsArmed();
	
	// Fade out the main rifle arms while in the air, so the jump state's arms show
	const float TargetWeight = bIsFalling ? 0.f : 1.f;
	RifleUpperBodyWeight = FMath::FInterpTo(RifleUpperBodyWeight, TargetWeight, DeltaSeconds, 10.f);
	
	const FRotator Delta = (Character->GetBaseAimRotation() - Character->GetActorRotation()).GetNormalized();
	AimPitch = Delta.Pitch;
	AimYaw = Delta.Yaw;

	
	if (bIsArmed)
	{
		AWeapon* Weapon = Character->GetEquippedWeapon();
		UStaticMeshComponent* WeaponMesh = Weapon ? Weapon->GetWeaponMesh() : nullptr;

		if (WeaponMesh && !WeaponMesh->DoesSocketExist(TEXT("LeftHandSocket")) && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Red, TEXT("LeftHandSocket NOT found on weapon mesh"));
		}
		
		if (WeaponMesh && WeaponMesh->DoesSocketExist(TEXT("LeftHandSocket")))
		{
			USkeletalMeshComponent* CharMesh = Character->GetMesh();
			const FTransform SocketWorld = WeaponMesh->GetSocketTransform(TEXT("LeftHandSocket"), RTS_World);

			// Where must the WRIST go so that the PALM lands on the gun socket?
			FTransform WristWorld = SocketWorld;
			if (CharMesh->DoesSocketExist(TEXT("LeftPalmSocket")))
			{
				const FTransform PalmLocal = CharMesh->GetSocketTransform(TEXT("LeftPalmSocket"), RTS_ParentBoneSpace);
				WristWorld = PalmLocal.Inverse() * SocketWorld;
			}

			// Convert to the character mesh's component space for the AnimGraph
			const FTransform& MeshToWorld = CharMesh->GetComponentTransform();
			LeftHandIKLocation = MeshToWorld.InverseTransformPosition(WristWorld.GetLocation());
			LeftHandIKRotation = MeshToWorld.InverseTransformRotation(WristWorld.GetRotation()).Rotator();
		}
	}
	
	// ---- TEMP DEBUG ----
	if (bIsArmed && GEngine)
	{
		AWeapon* W = Character->GetEquippedWeapon();
		UStaticMeshComponent* WM = W ? W->GetWeaponMesh() : nullptr;
		const bool bHasSocket = WM && WM->DoesSocketExist(TEXT("LeftHandSocket"));

		const FString Msg = FString::Printf(TEXT("Weapon: %s | Mesh: %s | Socket: %s | IK: %s"),
			W ? *W->GetName() : TEXT("NULL"),
			(WM && WM->GetStaticMesh()) ? *WM->GetStaticMesh()->GetName() : TEXT("NULL"),
			bHasSocket ? TEXT("yes") : TEXT("NO"),
			*LeftHandIKTransform.GetLocation().ToString());

		GEngine->AddOnScreenDebugMessage(2, 0.f, FColor::Yellow, Msg);

		if (bHasSocket)
		{
			DrawDebugSphere(Character->GetWorld(),
				WM->GetSocketLocation(TEXT("LeftHandSocket")), 5.f, 8, FColor::Green);
		}
	}
}
