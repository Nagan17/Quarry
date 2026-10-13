// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class AProjectile;

UCLASS()
class QUARRY_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();
	
	UStaticMeshComponent* GetWeaponMesh() const {return WeaponMesh; }
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon") 
	TSubclassOf<AProjectile> ProjectileClass;
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon") 
	FName MuzzleSocket = TEXT("MuzzleSocket");

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Fire(const FVector& AimPoint);
};
