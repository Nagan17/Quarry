// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class QUARRY_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();
	
	void InitBallistics(float Speed, float GravityScale);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, Category="Projectile|Mesh")
	UStaticMeshComponent* Mesh;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> SphereCollision;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> MovementComponent;
	
	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	float Damage = 60.0f;
	
	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	TSubclassOf<UDamageType> DamageType;
	
	UPROPERTY(VisibleAnywhere, Category="Projectile|FX")
	TObjectPtr<UNiagaraComponent> TrailComponent;
	
	UPROPERTY(EditDefaultsOnly, Category="Projectile|FX")
	TObjectPtr<UNiagaraSystem> ImpactFX;
	
	UPROPERTY(EditDefaultsOnly, Category="Projectile|FX", meta=(ClampMin=0.0f))
	float TrailFadeTime = 1.5f;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	private:
	void RetireProjectile();
	
	bool bHasHit = false;
};
