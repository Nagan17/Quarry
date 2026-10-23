// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/HUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UHUDWidget::SetHealth(float Current, float Max)
{
	const float New = Max > 0.f ? FMath::Clamp(Current / Max, 0.0f, 1.0f) : 0.0f;
	if (New < TargetHealth)
	{
		LagTimer = LagDelay;
	}
	
	if (New > LagHealth)
	{
		LagHealth = New;
	}
	
	TargetHealth = New;
}

void UHUDWidget::SetStamina(float Current, float Max)
{
	const float New = Max > 0.f ? FMath::Clamp(Current / Max, 0.f, 1.f) : 0.0f;
	if (New < 0.999f)
	{
		StaminaIdleTimer = StaminaFadeDelay;
	}
	
	if (PB_Stamina)
	{
		PB_Stamina->SetPercent(New);
	}
}

void UHUDWidget::SetAmmo(int32 InMag, int32 MagCapacity, int32 Reserve)
{
	if (RoundImages.Num() != MagCapacity)
	{
		RebuildRounds(MagCapacity);
	}
	
	for (int32 i = 0; i < RoundImages.Num(); i++)
	{
		const bool bLoaded = i < InMag;
		RoundImages[i]->SetBrush(MakeBrush(bLoaded ? RoundLoadedTex : RoundEmptyTex, RoundSize));
		RoundImages[i]->SetColorAndOpacity(bLoaded ? FLinearColor::White : RoundEmptyTint);
	}
	
	if (Txt_Reserve)
	{
		Txt_Reserve->SetText(FText::AsNumber(Reserve));
	}
}

void UHUDWidget::SetWeaponLabel(const FText& Label)
{
	if (Txt_Caliber)
	{
		Txt_Caliber->SetText(Label);
	}
}

void UHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (StaminaRow)
	{
		StaminaRow->SetRenderOpacity(0.0f);
	}
}

void UHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	DisplayHealth = FMath::FInterpTo(DisplayHealth, TargetHealth, InDeltaTime, HealthInterpSpeed);
	if (PB_Health)
	{
		PB_Health->SetPercent(DisplayHealth);
	}
	
	if (LagTimer > 0.0f)
	{
		LagTimer -= InDeltaTime;
	}
	else
	{
		LagHealth = FMath::FInterpTo(LagHealth, TargetHealth, InDeltaTime, LagInterpSpeed);
	}
	
	if (PB_Health)
	{
		PB_HealthLag->SetPercent(LagHealth);
	}
	
	if (PB_Health)
	{
		FLinearColor Fill = HealthColor;
		if (TargetHealth > 0.0f && TargetHealth <= LowHealthThreshold)
		{
			PulseTime += InDeltaTime;
			Fill.A = FMath::Lerp(0.45f, 1.0f, 0.5f + 0.5f * FMath::Sin(PulseTime * 6.0f));
		}
		else
		{
			PulseTime = 0.0f;
		}
		
		PB_Health->SetFillColorAndOpacity(Fill);
	}
	
	if (StaminaIdleTimer > 0.0f)
	{
		StaminaIdleTimer -= InDeltaTime;
	}
	
	const float TargetOpacity = StaminaIdleTimer > 0.0f ? 1.0f : 0.0f;
	StaminaOpacity = FMath::FInterpTo(StaminaOpacity, TargetOpacity, InDeltaTime, StaminaFadeSpeed);
	
	if (StaminaRow)
	{
		StaminaRow->SetRenderOpacity(StaminaOpacity);
	}
}

void UHUDWidget::RebuildRounds(int32 Capacity)
{
	if (!RoundsBox)
	{
		return;
	}
	
	RoundsBox->ClearChildren();
	RoundImages.Reset();
	
	for (int32 i = 0; i < Capacity; i++)
	{
		UOverlay* Cell = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
		
		if (RoundShadowTex)
		{
			UImage* Shadow = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
			Shadow->SetBrush(MakeBrush(RoundShadowTex, RoundSize * FVector2D(2.f, 1.25f)));
			Shadow->SetRenderTranslation(FVector2D(1.f, 2.f));
			UOverlaySlot* S = Cell->AddChildToOverlay(Shadow);
			S->SetHorizontalAlignment(HAlign_Center);
			S->SetVerticalAlignment(VAlign_Center);
		}
		
		UImage* Round = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		UOverlaySlot* R = Cell->AddChildToOverlay(Round);
		R->SetHorizontalAlignment(HAlign_Center);
		R->SetVerticalAlignment(VAlign_Center);
		RoundImages.Add(Round);

		UHorizontalBoxSlot* HSlot = RoundsBox->AddChildToHorizontalBox(Cell);
		HSlot->SetVerticalAlignment(VAlign_Bottom);
	}
}

FSlateBrush UHUDWidget::MakeBrush(UTexture2D* Tex, const FVector2D& Size)
{
	FSlateBrush Brush;
	Brush.SetResourceObject(Tex);
	Brush.ImageSize = Size;
	return Brush;
}
