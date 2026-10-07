// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "GameFramework/CharacterMovementComponent.h"


APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = DefaultArmLength;
	CameraBoom->bUsePawnControlRotation = true;
	
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	BoomBaseLocation = CameraBoom->GetRelativeLocation();
	UpdateMovementSpeed();
}

void APlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	const float TargetLength = bIsAiming ? AimArmLength : DefaultArmLength;
	const FVector TargetOffset = bIsAiming ? AimSocketOffset : FVector::ZeroVector;
	
	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetLength, DeltaSeconds, CameraInterpSpeed);
	CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetOffset, DeltaSeconds, CameraInterpSpeed);
	
	// Smooth crouch camera
	CrouchCameraOffset = FMath::FInterpTo(CrouchCameraOffset, 0.f, DeltaSeconds, CrouchCameraInterpSpeed);
	CameraBoom->SetRelativeLocation(BoomBaseLocation + FVector(0.f, 0.f, CrouchCameraOffset));
}

void APlayerCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
	
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UEnhancedInputComponent* Input = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	
	Input->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayerCharacter::JumpPressed);
	Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	
	Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
	Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
	
	if (SprintAction)
	{
		Input->BindAction(SprintAction, ETriggerEvent::Started,   this, &APlayerCharacter::SprintStarted);
		Input->BindAction(SprintAction, ETriggerEvent::Completed, this, &APlayerCharacter::SprintCompleted);
	}
	
	if (AimAction)
	{
		Input->BindAction(AimAction, ETriggerEvent::Started, this, &APlayerCharacter::AimStarted);
		Input->BindAction(AimAction, ETriggerEvent::Completed, this, &APlayerCharacter::AimCompleted);
	}
	
	if (FireAction)
	{
		Input->BindAction(FireAction, ETriggerEvent::Started, this, &APlayerCharacter::FirePressed);
	}
	
	if (EquipAction)
	{
		Input->BindAction(EquipAction, ETriggerEvent::Started, this, &APlayerCharacter::EquipPressed);
	}
	
	if (CrouchAction)
	{
		Input->BindAction(CrouchAction, ETriggerEvent::Started, this, &APlayerCharacter::CrouchPressed);
	}
}

void APlayerCharacter::SetAiming(bool bNewAiming)
{
	Super::SetAiming(bNewAiming);
}

void APlayerCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	
	CrouchCameraOffset += ScaledHalfHeightAdjust;
}

void APlayerCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	
	CrouchCameraOffset -= ScaledHalfHeightAdjust;

}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (!Controller) return;
	
	const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
	AddMovementInput(Forward, Axis.Y);
	AddMovementInput(Right, Axis.X);
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void APlayerCharacter::AimStarted()
{
	SetAiming(true);
}

void APlayerCharacter::AimCompleted()
{
	SetAiming(false);
}

void APlayerCharacter::FirePressed()
{
	Fire();
}

void APlayerCharacter::SprintStarted()
{
	SetSprinting(true);
}

void APlayerCharacter::SprintCompleted()
{
	SetSprinting(false);
}

void APlayerCharacter::EquipPressed()
{
	ToggleArmed();
}

void APlayerCharacter::CrouchPressed()
{
	ToggleCrouch();
}

void APlayerCharacter::JumpPressed()
{
	if (GetCharacterMovement()->bWantsToCrouch)
	{
		SetCrouching(false);
		return;
	}
	
	Jump();
}
