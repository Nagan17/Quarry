// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Projectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/BloodComponent.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SphereCollision = CreateDefaultSubobject<USphereComponent>("SphereCollision");
	SphereCollision->InitSphereRadius(1.5f);
	SphereCollision->SetCollisionProfileName(TEXT("Projectile"));
	SphereCollision->bReturnMaterialOnMove = true;
	SetRootComponent(SphereCollision);
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(SphereCollision);
	
	TrailComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailComponent"));
	TrailComponent->SetupAttachment(SphereCollision);
	TrailComponent->bAutoActivate = true;
	
	
	MovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComponent"));
	MovementComponent->UpdatedComponent = SphereCollision;
	MovementComponent->InitialSpeed = 60000.0f;
	MovementComponent->MaxSpeed = 0;
	MovementComponent->ProjectileGravityScale = 1.0f;
	MovementComponent->bRotationFollowsVelocity = true;
	MovementComponent->bShouldBounce = false;
	MovementComponent->bForceSubStepping = true;
	
	InitialLifeSpan = 5.0f;
}

void AProjectile::InitBallistics(float Speed, float GravityScale)
{
	MovementComponent->InitialSpeed = Speed;
	MovementComponent->ProjectileGravityScale = GravityScale;
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	SphereCollision->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	
	if (GetOwner())
	{
		SphereCollision->IgnoreActorWhenMoving(GetOwner(), true);
	}
	
	if (GetInstigator())
	{
		SphereCollision->IgnoreActorWhenMoving(GetInstigator(), true);
	}

}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (bHasHit) return;
	
	bHasHit = true;
	
	if (OtherActor && OtherActor != this)
	{
		UGameplayStatics::ApplyPointDamage(OtherActor, Damage, GetVelocity().GetSafeNormal(),
			Hit, GetInstigatorController(), this, DamageType);
	}
	
	SpawnImpactFX(OtherActor, Hit);
	
	SetActorLocation(Hit.ImpactPoint, false, nullptr, ETeleportType::TeleportPhysics);
	RetireProjectile();
}

void AProjectile::SpawnImpactFX(const AActor* OtherActor, const FHitResult& Hit) const
{
	if (OtherActor && OtherActor->FindComponentByClass<UBloodComponent>())
	{
		return;
	}
	
	UNiagaraSystem* FX = ImpactFX;
	const EPhysicalSurface Surface = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
	if (const TObjectPtr<UNiagaraSystem>* Found = ImpactFXBySurface.Find(Surface))
	{
		FX = *Found;
	}
	
	if (FX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), FX, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
	}
}

void AProjectile::RetireProjectile()
{
	MovementComponent->StopMovementImmediately();
	MovementComponent->Deactivate();
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetVisibility(false);
	
	TrailComponent->Deactivate();
	
	SetLifeSpan(FMath::Max(TrailFadeTime, 0.01f));
}

