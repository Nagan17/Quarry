// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDWidget.generated.h"

class UProgressBar;
class UHorizontalBox;
class UTextBlock;
class UImage;
class UTexture2D;

/**
 * 
 */
UCLASS()
class QUARRY_API UHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetHealth(float Current, float Max);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetStamina(float Current, float Max);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetAmmo(int32 InMag, int32 MagCapacity, int32 Reserve);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetWeaponLabel(const FText& Label);
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	//Bound widgets
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UProgressBar> PB_Health;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UProgressBar> PB_HealthLag;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UProgressBar> PB_Stamina;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UWidget> StaminaRow;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UHorizontalBox> RoundsBox;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Txt_Reserve;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Txt_Caliber;
	
	UPROPERTY(EditDefaultsOnly, Category = "HUD|Ammo") TObjectPtr<UTexture2D> RoundLoadedTex;
	UPROPERTY(EditDefaultsOnly, Category = "HUD|Ammo") TObjectPtr<UTexture2D> RoundEmptyTex;
	UPROPERTY(EditDefaultsOnly, Category = "HUD|Ammo") TObjectPtr<UTexture2D> RoundShadowTex;
	UPROPERTY(EditDefaultsOnly, Category = "HUD|Ammo") FVector2D RoundSize = FVector2D(10.f, 36.f);
	UPROPERTY(EditDefaultsOnly, Category = "HUD|Ammo") FLinearColor RoundEmptyTint = FLinearColor(1.f, 1.f, 1.f, 0.7f);
	
	UPROPERTY(EditDefaultsOnly, Category = "HUD|Vitals") float HealthInterpSpeed = 12.f;
	UPROPERTY(EditDefaultsOnly, Category = "HUD|Vitals") float LagDelay = 0.5f;
	UPROPERTY(EditDefaultsOnly, Category = "HUD|Vitals") float LagInterpSpeed = 2.5f;
	UPROPERTY(EditDefaultsOnly, Category = "HUD|Vitals") float StaminaFadeDelay = 1.5f;
	UPROPERTY(EditDefaultsOnly, Category = "HUD|Vitals") float StaminaFadeSpeed = 4.f;
	UPROPERTY(EditDefaultsOnly, Category = "HUD|Vitals") float LowHealthThreshold = 0.25f;
	UPROPERTY(EditDefaultsOnly, Category = "HUD|Vitals") FLinearColor HealthColor = FLinearColor::FromSRGBColor(FColor(0xEE, 0x5C, 0x50));
	
private:
	void RebuildRounds(int32 Capacity);
	static FSlateBrush MakeBrush(UTexture2D* Tex, const FVector2D& Size);

	UPROPERTY() TArray<TObjectPtr<UImage>> RoundImages;
	
	float TargetHealth = 1.f;
	float DisplayHealth = 1.f;
	float LagHealth = 1.f;
	float LagTimer = 0.f;
	float StaminaIdleTimer = 0.f;
	float StaminaOpacity = 0.f;
	float PulseTime = 0.f;
};
