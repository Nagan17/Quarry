// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Projectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SphereCollision = CreateDefaultSubobject<USphereComponent>("SphereCollision");
	SphereCollision->InitSphereRadius(1.5f);
	SphereCollision->SetCollisionProfileName(TEXT("Projectile"));
	SetRootComponent(SphereCollision);
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(SphereCollision);
	
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
	if (OtherActor && OtherActor != this)
	{
		UGameplayStatics::ApplyPointDamage(OtherActor, Damage, GetVelocity().GetSafeNormal(),
			Hit, GetInstigatorController(), this, DamageType);
	}
	
	Destroy();
}

