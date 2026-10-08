// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Weapons/Weapon.h"


APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
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
	
	DefaultFOV       = FollowCamera->FieldOfView;
	DefaultCamRelLoc = FollowCamera->GetRelativeLocation();
	
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
	
	UpdateADSCamera(DeltaSeconds);
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
		Input->BindAction(AimAction, ETriggerEvent::Started, this, &APlayerCharacter::AimToggle);
		//Input->BindAction(AimAction, ETriggerEvent::Completed, this, &APlayerCharacter::AimCompleted);
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
	const bool bWas = bIsAiming;
	Super::SetAiming(bNewAiming);
	if (bIsAiming == bWas) return;
	
	if (bIsAiming)
	{
		StartADS();
	}
	else
	{
		StopADS();
	}
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

void APlayerCharacter::StartADS()
{
	ADSDuration = (ADSMontage ? ADSMontage->GetPlayLength() : ADSBlendTime);
	ADSDuration = FMath::Max(ADSDuration, 0.05f);
	ADSState = EADSState::Entering;
}

void APlayerCharacter::StopADS()
{
	if (ADSState == EADSState::Scoped) ExitScope();
	if (ADSState != EADSState::Hip) ADSState = EADSState::Exiting;
}

void APlayerCharacter::EnterScope()
{
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
	FString::Printf(TEXT("EnterScope | WidgetClass: %s"),
	ScopeWidgetClass ? *ScopeWidgetClass->GetName() : TEXT("NONE")));
	
	ADSState = EADSState::Scoped;

	const FRotator YawRot(0.f, GetControlRotation().Yaw, 0.f);
	ScopedYawSpaceOffset = YawRot.UnrotateVector(FollowCamera->GetComponentLocation() - GetActorLocation());

	FollowCamera->SetFieldOfView(ScopedFOV);
	SetFirstPersonHidden(true);

	if (!ScopeWidget && ScopeWidgetClass)
	{
		ScopeWidget = CreateWidget<UUserWidget>(Cast<APlayerController>(GetController()), ScopeWidgetClass);
	}
	
	if (ScopeWidget)
	{
		ScopeWidget->AddToViewport();
	}
}

void APlayerCharacter::ExitScope()
{
	if (ScopeWidget)
	{
		ScopeWidget->RemoveFromParent();
	}
	
	SetFirstPersonHidden(false);
	FollowCamera->SetFieldOfView(DefaultFOV * EnterFOVScale);
}

void APlayerCharacter::UpdateADSCamera(float DeltaTime)
{
	GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Yellow,
	FString::Printf(TEXT("ADS State: %d  Alpha: %.2f  Aiming: %d"),
	(int32)ADSState, ADSAlpha, bIsAiming));
	
	if (ADSState == EADSState::Hip) return;

	if (ADSState == EADSState::Scoped)
	{
		const FRotator YawRot(0.f, GetControlRotation().Yaw, 0.f);
		FollowCamera->SetWorldLocation(GetActorLocation() + YawRot.RotateVector(ScopedYawSpaceOffset));
		return;
	}

	const float Dir = (ADSState == EADSState::Entering) ? 1.f : -1.f;
	ADSAlpha = FMath::Clamp(ADSAlpha + Dir * DeltaTime / ADSDuration, 0.f, 1.f);

	if (ADSState == EADSState::Exiting && ADSAlpha <= 0.f)
	{
		ADSState = EADSState::Hip;
		FollowCamera->SetRelativeLocation(DefaultCamRelLoc);
		FollowCamera->SetFieldOfView(DefaultFOV);
		return;
	}

	const FVector HipLoc = CameraBoom->GetSocketTransform(USpringArmComponent::SocketName)
									 .TransformPosition(DefaultCamRelLoc);
	FVector ScopeLoc = HipLoc;
	if (EquippedWeapon)
	{
		if (UStaticMeshComponent* WM = EquippedWeapon->GetWeaponMesh(); WM && WM->DoesSocketExist(ScopeEyeSocket))
		{
			ScopeLoc = WM->GetSocketLocation(ScopeEyeSocket);
		}
	}
	
	const float A = FMath::InterpEaseInOut(0.f, 1.f, ADSAlpha, 2.f);
	FollowCamera->SetWorldLocation(FMath::Lerp(HipLoc, ScopeLoc, A));
	FollowCamera->SetFieldOfView(FMath::Lerp(DefaultFOV, DefaultFOV * EnterFOVScale, A));

	if (ADSState == EADSState::Entering && ADSAlpha >= 1.f)
	{
		EnterScope();
	}
}

void APlayerCharacter::SetFirstPersonHidden(bool bHide)
{
	GetMesh()->SetOwnerNoSee(bHide);
	
	if (EquippedWeapon)
	{
		if (UStaticMeshComponent* WM = EquippedWeapon->GetWeaponMesh())
		{
			WM->SetOwnerNoSee(bHide);
		}
	}
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
	
	const float Sens = (ADSState == EADSState::Scoped) ? (ScopedFOV / DefaultFOV) * ScopedSensitivityMultiplier : 1.f;

	AddControllerYawInput(Axis.X * Sens);
	AddControllerPitchInput(Axis.Y * Sens);
}

void APlayerCharacter::AimToggle(const FInputActionValue& Value)
{
	SetAiming(!bIsAiming);
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
	SetAiming(false);
	
	if (GetCharacterMovement()->bWantsToCrouch)
	{
		SetCrouching(false);
		return;
	}
	
	Jump();
}
