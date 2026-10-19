// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "Characters/BaseCharacter.h"
#include "Quarry/Characters/BaseCharacter.h"
#include "InputActionValue.h"
#include "Blueprint/UserWidget.h"
#include "PlayerCharacter.generated.h"

class UCrosshairWidget;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class USceneCaptureComponent2D;

UENUM()
enum class EADSState : uint8 
{
	Hip, 
	Entering, 
	Scoped, 
	Exiting
};

/**
 * 
 */
UCLASS()
class QUARRY_API APlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()
	
public:
	APlayerCharacter();
	
	virtual void Tick(float DeltaSeconds) override;
	
protected:
	virtual void BeginPlay() override;
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void SetAiming(bool bNewAiming) override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual FVector GetAimPoint() const override;
	virtual TOptional<FVector> GetShotOriginOverride() const override;
	virtual void SetArmed(bool bNewArmed) override;
	
	//Crosshair
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCrosshairWidget> CrosshairWidgetClass;
	
	UPROPERTY() 
	TObjectPtr<UCrosshairWidget> CrosshairWidget;
	
	void UpdateCrosshair();
	
	//ADS Scope
	UPROPERTY(EditDefaultsOnly, Category = "ADS") 
	float ADSBlendTime = 0.3f;
	
	UPROPERTY(EditDefaultsOnly, Category = "ADS") 
	float ScopedFOV = 20.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "ADS") 
	float EnterFOVScale = 0.85f;
	
	UPROPERTY(EditDefaultsOnly, Category = "ADS") 
	float ScopedSensitivityMultiplier = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "ADS") 
	float ADSDuration = 0.3f;
	
	UPROPERTY(EditDefaultsOnly, Category = "ADS") 
	FName ScopeEyeSocket = TEXT("ScopeEyeSocket");
	
	UPROPERTY(EditDefaultsOnly, Category = "ADS") 
	TSubclassOf<UUserWidget> ScopeWidgetClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ADS")
	TObjectPtr<USceneCaptureComponent2D> ScopeCapture;
	
	UPROPERTY(EditDefaultsOnly, Category = "ADS")
	float ScopeSpawnForwardOffset = 30.f;
	
	UPROPERTY() 
	TObjectPtr<UUserWidget> ScopeWidget;
	
	// Scope zoom
	UPROPERTY(EditDefaultsOnly, Category = "ADS") 
	float MinScopeFOV = 5.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "ADS") 
	float ZoomStepMultiplier = 1.25f;
	
	UPROPERTY(EditDefaultsOnly, Category = "ADS") 
	float ZoomInterpSpeed = 12.f;

	float TargetScopeFOV = 20.f;

	EADSState ADSState = EADSState::Hip;
	float   ADSAlpha = 0.f;

	float   DefaultFOV = 90.f;
	FVector DefaultCamRelLoc;
	FVector ScopedYawSpaceOffset;

	void StartADS();
	void StopADS();
	void EnterScope();
	void ExitScope();
	void UpdateADSCamera(float DeltaTime);
	void SetFirstPersonHidden(bool bHide);
	void ZoomInput(const FInputActionValue& Value);
	
	//Scope Sway
	UPROPERTY(EditDefaultsOnly, Category = "ADS|Sway")
	float IdleSwayAmount = 0.12f;
	
	UPROPERTY(EditDefaultsOnly, Category = "ADS|Sway")
	float IdleSwaySpeed  = 0.8f;

	UPROPERTY(EditDefaultsOnly, Category = "ADS|Sway")
	float MoveSwayAmount = 0.7f;
	
	UPROPERTY(EditDefaultsOnly, Category = "ADS|Sway")
	float MoveSwaySpeed  = 7.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "ADS|Sway")
	float CrouchSwayMultiplier = 0.5f;
	
	UPROPERTY(EditDefaultsOnly, Category = "ADS|Sway")
	float SwayBlendSpeed = 6.0f;

	UPROPERTY(EditDefaultsOnly, Category = "ADS|Sway")
	float CycleShakeAmount    = 0.6f;
	
	UPROPERTY(EditDefaultsOnly, Category = "ADS|Sway")
	float CycleShakeFrequency = 12.0f;

	UPROPERTY(EditDefaultsOnly, Category = "ADS|Sway")
	float WidgetPixelsPerDegree = 40.f;

	UPROPERTY(EditDefaultsOnly, Category = "ADS|Sway")
	float WidgetRollScale = 1.f;
	
	float IdlePhase = 0.f;
	float MovePhase = 0.f;
	float ShakeTime = 0.f;
	float MoveSwayAlpha = 0.f;
	float CycleShakeAlpha = 0.f;

	void UpdateScopeSway(float DeltaTime);
	void ResetScopeSway();

	void ApplyScopeWidgetSway(const FRotator& Sway);
	
	//Input Handler
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void AimToggle(const FInputActionValue& Value);
	void FirePressed();
	void ReloadPressed();
	void SprintStarted();
	void SprintCompleted();
	void EquipPressed();
	void CrouchPressed();
	void JumpPressed();
	
	//Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintreadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;
	
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float DefaultArmLength = 400.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float AimArmLength = 180.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	FVector AimSocketOffset = FVector(0.0f, 60.0f, 40.0f);
	
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float CameraInterpSpeed = 10.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float CrouchCameraInterpSpeed = 8.f;

	float CrouchCameraOffset = 0.f;
	FVector BoomBaseLocation = FVector::ZeroVector;
	
	//Input Asset
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
 
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;
 
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;
 
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> SprintAction;
 
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> AimAction;
 
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> FireAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> ReloadAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> EquipAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> CrouchAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> ZoomAction;
};
