// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CrosshairWidget.generated.h"

/**
 * 
 */

class UImage;
class UTexture2D;

UCLASS()
class QUARRY_API UCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()
	
	public:
	void SetArmedStyle(bool bArmed);
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> CrosshairImage;
	
	UPROPERTY(EditDefaultsOnly, Category = "Crosshair")
	TObjectPtr<UTexture2D> NormalCHTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Crosshair")
	TObjectPtr<UTexture2D> ArmedCHTexture;
};
