// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/CrosshairWidget.h"

#include "Components/Image.h"

void UCrosshairWidget::SetArmedStyle(bool bArmed)
{
	if (!CrosshairImage) return;
	
	if (UTexture2D* Text = bArmed ? ArmedCHTexture : NormalCHTexture)
	{
		CrosshairImage->SetBrushFromTexture(Text, true);
	}
}
