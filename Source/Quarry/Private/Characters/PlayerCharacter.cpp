// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Components/SceneCaptureComponent2D.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Weapons/Weapon.h"
#include "Widgets/CrosshairWidget.h"
#include "Camera/CameraShakeBase.h"
#include "Components/HealthComponent.h"
#include "Widgets/HUDWidget.h"

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
	
	//Scope Render Target
	ScopeCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("ScopeCapture"));
	ScopeCapture->SetupAttachment(FollowCamera);
	ScopeCapture->bCaptureEveryFrame = false;
	ScopeCapture->bCaptureOnMovement = false;
	ScopeCapture->bAlwaysPersistRenderingState = true;
	ScopeCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	EquippedWeapon->OnAmmoChanged.AddUniqueDynamic(this, &APlayerCharacter::OnAmmoChanged);
	
	BoomBaseLocation = CameraBoom->GetRelativeLocation();
	
	DefaultFOV       = FollowCamera->FieldOfView;
	DefaultCamRelLoc = FollowCamera->GetRelativeLocation();
	
	ScopeCapture->FOVAngle = ScopedFOV;
	
	Stamina = MaxStamina;
	
	CreateHUD();
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
	UpdateStamina(DeltaSeconds);
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
		
		if (!CrosshairWidget && CrosshairWidgetClass && IsLocallyControlled())
		{
			CrosshairWidget = CreateWidget<UCrosshairWidget>(PC, CrosshairWidgetClass);
			CrosshairWidget->AddToViewport();
		}
	}
	
	UpdateCrosshair();
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
	
	if (ReloadAction)
	{
		Input->BindAction(ReloadAction, ETriggerEvent::Started, this, &APlayerCharacter::ReloadPressed);
	}
	
	if (EquipAction)
	{
		Input->BindAction(EquipAction, ETriggerEvent::Started, this, &APlayerCharacter::EquipPressed);
	}
	
	if (CrouchAction)
	{
		Input->BindAction(CrouchAction, ETriggerEvent::Started, this, &APlayerCharacter::CrouchPressed);
	}
	
	if (ZoomAction)
	{
		Input->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &APlayerCharacter::ZoomInput);
	}
}

void APlayerCharacter::SetAiming(bool bNewAiming)
{
	if (bNewAiming && EquippedWeapon && EquippedWeapon->GetState() == EWeaponState::Reloading) return;

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
	
	UpdateCrosshair();
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

FVector APlayerCharacter::GetAimPoint() const
{
	const USceneComponent* View = (ADSState == EADSState::Scoped)
			? static_cast<const USceneComponent*>(ScopeCapture.Get())
			: static_cast<const USceneComponent*>(FollowCamera.Get());

	const FVector Fwd = View->GetForwardVector();
	FVector Start = View->GetComponentLocation();

	const float ToChar = FVector::DotProduct(GetActorLocation() - Start, Fwd);
	Start += Fwd * FMath::Max(ToChar, 0.f);
	const FVector End = Start + Fwd * 100000.f;

	FCollisionQueryParams Q(SCENE_QUERY_STAT(AimTrace), false, this);
	Q.AddIgnoredActor(EquippedWeapon);

	FHitResult Hit;
	return GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Q) ? Hit.ImpactPoint : End;
}

TOptional<FVector> APlayerCharacter::GetShotOriginOverride() const
{
	if (ADSState != EADSState::Scoped)
	{
		return{};
	}
	
	const FVector CamLoc = ScopeCapture->GetComponentLocation();
	const FVector Desired = CamLoc + ScopeCapture->GetForwardVector() * ScopeSpawnForwardOffset;
	
	FHitResult Hit;
	FCollisionQueryParams Q(SCENE_QUERY_STAT(ScopeSpawn), false, this);
	Q.AddIgnoredActor(EquippedWeapon);
	if (GetWorld()->LineTraceSingleByChannel(Hit, CamLoc, Desired, ECC_Visibility, Q))
	{
		return Hit.Location;
	}
	
	return Desired;
}

void APlayerCharacter::SetArmed(bool bNewArmed)
{
	Super::SetArmed(bNewArmed);
	UpdateCrosshair();
}

void APlayerCharacter::OnAmmoChanged(bool bChambered, int32 MagAmmo, int32 ReserveAmmo)
{
	Super::OnAmmoChanged(bChambered, MagAmmo, ReserveAmmo);
	
	RefreshAmmoHUD();
}

void APlayerCharacter::HandleDeath(UHealthComponent* HealthComponent, AController* Contr)
{
	if (bIsDead) return;
	
	Super::HandleDeath(HealthComponent, Contr);
	
	bSprintHeld = false;
	ApplySprintState(false);
	
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
	}

	UpdateCrosshair();
}

void APlayerCharacter::OnHurt(float DamageTaken)
{
	if (!HurtCameraShake) return;
	
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		const float Scale = FMath::Clamp(DamageTaken / HurtShakeDamageScale, 0.3f, 1.5f);
		PC->ClientStartCameraShake(HurtCameraShake, Scale);
	}
}

void APlayerCharacter::UpdateCrosshair()
{
	if (!CrosshairWidget) return;
	
	if (bIsDead)
	{
		CrosshairWidget->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	
	if (bIsAiming)
	{
		CrosshairWidget->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	
	CrosshairWidget->SetArmedStyle(bIsArmed);
	CrosshairWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void APlayerCharacter::StartADS()
{
	ADSDuration = FMath::Max(ADSBlendTime, 0.05f);
	ADSState = EADSState::Entering;
}

void APlayerCharacter::StopADS()
{
	if (ADSState == EADSState::Scoped) ExitScope();
	if (ADSState != EADSState::Hip) ADSState = EADSState::Exiting;
}

void APlayerCharacter::EnterScope()
{
	ADSState = EADSState::Scoped;

	const FRotator YawRot(0.f, GetControlRotation().Yaw, 0.f);
	ScopedYawSpaceOffset = YawRot.UnrotateVector(FollowCamera->GetComponentLocation() - GetActorLocation());
	
	SetFirstPersonHidden(true);
	ScopeCapture->HideActorComponents(this);
	if (EquippedWeapon)
	{
		ScopeCapture->HideActorComponents(EquippedWeapon);
	}
	
	TargetScopeFOV = ScopedFOV;
	ScopeCapture->FOVAngle = ScopedFOV;
	ScopeCapture->bCaptureEveryFrame = true;
	
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
	
	ScopeCapture->bCaptureEveryFrame = false;
	SetFirstPersonHidden(false);
	ResetScopeSway();
}

void APlayerCharacter::UpdateADSCamera(float DeltaTime)
{
	if (ADSState == EADSState::Hip) return;

	if (ADSState == EADSState::Scoped)
	{
		const FRotator YawRot(0.f, GetControlRotation().Yaw, 0.f);
		FollowCamera->SetWorldLocation(GetActorLocation() + YawRot.RotateVector(ScopedYawSpaceOffset));
		
		ScopeCapture->FOVAngle = FMath::FInterpTo(ScopeCapture->FOVAngle, TargetScopeFOV, DeltaTime, ZoomInterpSpeed);
		UpdateScopeSway(DeltaTime);
		
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

void APlayerCharacter::ZoomInput(const FInputActionValue& Value)
{
	if (ADSState != EADSState::Scoped) return;

	const float Wheel = Value.Get<float>();
	if (FMath::IsNearlyZero(Wheel)) return;
	
	if (Wheel > 0.0f)
	{
		TargetScopeFOV = TargetScopeFOV / ZoomStepMultiplier;
	}
	else
	{
		TargetScopeFOV = TargetScopeFOV * ZoomStepMultiplier;
	}
	
	TargetScopeFOV = FMath::Clamp(TargetScopeFOV, MinScopeFOV, ScopedFOV);
}

void APlayerCharacter::UpdateScopeSway(float DeltaTime)
{
	float MoveTarget = FMath::Clamp(GetVelocity().Size2D() / FMath::Max(AimWalkSpeed, 1.f), 0.f, 1.f);
	if (bIsCrouched)
	{
		MoveTarget *= CrouchSwayMultiplier;
	}
	
	MoveSwayAlpha = FMath::FInterpTo(MoveSwayAlpha, MoveTarget, DeltaTime, SwayBlendSpeed);

	const bool bCycling = EquippedWeapon && EquippedWeapon->GetState() == EWeaponState::Cycling;
	CycleShakeAlpha = FMath::FInterpTo(CycleShakeAlpha, bCycling ? 1.f : 0.f, DeltaTime, 10.f);

	// Accumulate phase (not time * speed) so changing speed never causes a jump
	IdlePhase += DeltaTime * IdleSwaySpeed;
	MovePhase += DeltaTime * MoveSwaySpeed;
	ShakeTime += DeltaTime;

	// Idle breathing: figure-8 (pitch at twice the yaw frequency)
	float Yaw   = FMath::Sin(IdlePhase)       * IdleSwayAmount;
	float Pitch = FMath::Sin(IdlePhase * 2.f) * IdleSwayAmount * 0.5f;

	// Walking: side-to-side with a footstep bob
	Yaw   += FMath::Sin(MovePhase)       * MoveSwayAmount * MoveSwayAlpha;
	Pitch += FMath::Sin(MovePhase * 2.f) * MoveSwayAmount * 0.5f * MoveSwayAlpha;

	// Bolt cycle: jittery noise (+ slight cant)
	const float T = ShakeTime * CycleShakeFrequency;
	Yaw   += FMath::PerlinNoise1D(T)         * CycleShakeAmount * CycleShakeAlpha;
	Pitch += FMath::PerlinNoise1D(T + 37.f)  * CycleShakeAmount * CycleShakeAlpha;
	const float Roll = FMath::PerlinNoise1D(T + 91.f) * CycleShakeAmount * 2.f * CycleShakeAlpha;

	const FRotator Sway(Pitch, Yaw, Roll);
	ScopeCapture->SetRelativeRotation(Sway);
	ApplyScopeWidgetSway(Sway);
}

void APlayerCharacter::ResetScopeSway()
{
	MoveSwayAlpha = 0.0f;
	CycleShakeAlpha = 0.0f;
	ScopeCapture->SetRelativeRotation(FRotator::ZeroRotator);
	ApplyScopeWidgetSway(FRotator::ZeroRotator);
}

void APlayerCharacter::ApplyScopeWidgetSway(const FRotator& Sway)
{
	if (!ScopeWidget) return;

	ScopeWidget->SetRenderTranslation(FVector2D(Sway.Yaw, -Sway.Pitch) * WidgetPixelsPerDegree);
	ScopeWidget->SetRenderTransformAngle(Sway.Roll * WidgetRollScale);
}

void APlayerCharacter::CreateHUD()
{
	if (HUDWidget || !HUDWidgetClass || !IsLocallyControlled()) { return; }

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}

	HUDWidget = CreateWidget<UHUDWidget>(PC, HUDWidgetClass);
	HUDWidget->AddToViewport();

	HUDWidget->SetHealth(HealthComp->GetHealth(), HealthComp->GetMaxHealth());
	HUDWidget->SetStamina(Stamina, MaxStamina);
	RefreshAmmoHUD();
}

void APlayerCharacter::RefreshAmmoHUD()
{
	if (!HUDWidget) { return; }
	HUDWidget->SetAmmo(EquippedWeapon->GetCurrentAmmo(), EquippedWeapon->GetMaxCapacity(), EquippedWeapon->GetReserveAmmo());
	HUDWidget->SetWeaponLabel(FText::FromString(TEXT(".308")));
}

void APlayerCharacter::HandleHealthChanged(UHealthComponent* HealthComponent, float NewHealth, float Delta)
{
	Super::HandleHealthChanged(HealthComponent, NewHealth, Delta);
	if (HUDWidget)
	{
		HUDWidget->SetHealth(NewHealth, HealthComp->GetMaxHealth());
	}
}

void APlayerCharacter::UpdateStamina(float DeltaTime)
{
	if (GetCharacterMovement()->IsMovingOnGround() || !bSprintActive)
	{
		ApplySprintState(bSprintHeld && CanSprintNow());
	}

	if (IsActuallySprinting())
	{
		ConsumeStamina(SprintDrainPerSec * DeltaTime);
		return;
	}

	if (RegenDelayTimer > 0.f)
	{
		RegenDelayTimer -= DeltaTime;
		return;
	}

	if (Stamina < MaxStamina)
	{
		Stamina = FMath::Min(Stamina + RegenPerSec * DeltaTime, MaxStamina);

		if (bExhausted && Stamina >= MinStaminaToSprint)
		{
			bExhausted = false;
		}
		PushStaminaToHUD();
	}
}

bool APlayerCharacter::CanSprintNow() const
{
	if (bIsDead || bExhausted || bIsAiming || bIsCrouched) return false;
	if (!GetCharacterMovement()->IsMovingOnGround()) return false;
	return Stamina > 0.f;
}

bool APlayerCharacter::IsActuallySprinting() const
{
	return bSprintActive
	&& GetCharacterMovement()->IsMovingOnGround()
	&& GetVelocity().Size2D() > 10.f;
}

void APlayerCharacter::ConsumeStamina(float Amount)
{
	if (Amount <= 0.f) return;

	Stamina = FMath::Max(Stamina - Amount, 0.f);
	RegenDelayTimer = RegenDelay;

	if (Stamina <= 0.f)
	{
		bExhausted = true;
		RegenDelayTimer = ExhaustedRegenDelay;
		ApplySprintState(false);
	}
	PushStaminaToHUD();
}

void APlayerCharacter::ApplySprintState(bool bShouldSprint)
{
	if (bSprintActive == bShouldSprint) return;
	bSprintActive = bShouldSprint;
	SetSprinting(bShouldSprint);
}

void APlayerCharacter::PushStaminaToHUD()
{
	if (HUDWidget)
	{
		HUDWidget->SetStamina(Stamina, MaxStamina);
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
	
	const float Sens = (ADSState == EADSState::Scoped) ? (ScopeCapture->FOVAngle / DefaultFOV) * ScopedSensitivityMultiplier : 1.f;

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

void APlayerCharacter::ReloadPressed()
{
	Reload();
}

void APlayerCharacter::SprintStarted()
{
	bSprintHeld = true;
}

void APlayerCharacter::SprintCompleted()
{
	bSprintHeld = false;
	ApplySprintState(false);
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
	
	if (JumpStaminaCost > 0.f)
	{
		if (Stamina < JumpStaminaCost || !GetCharacterMovement()->IsMovingOnGround()) return;
		ConsumeStamina(JumpStaminaCost);
	}
	
	Jump();
}
