// Fill out your copyright notice in the Description page of Project Settings.


#include "Animals/BaseAnimal.h"
#include <Components/HealthComponent.h>
#include "BrainComponent.h"
#include "Animals/AnimalAIController.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AISense_Damage.h"

// Sets default values
ABaseAnimal::ABaseAnimal()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	
	AIControllerClass = AAnimalAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	bUseControllerRotationYaw = false;
	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->bOrientRotationToMovement = true;
	Move->bUseControllerDesiredRotation = false;
	Move->RotationRate = FRotator(0, 360, 0);
}

void ABaseAnimal::SetRunning(bool bRun)
{
	if (!AnimalData) return;
	GetCharacterMovement()->MaxWalkSpeed = bRun ? AnimalData->RunSpeed : AnimalData->WalkSpeed;
}

void ABaseAnimal::StartLyingDown()
{
	if (Posture != EAnimalPosture::Standing) return;
	
	GetCharacterMovement()->StopMovementImmediately();
	bIsGrazing = false;
	SetPosture(EAnimalPosture::LyingDown);
	
	const float Duration = PlayPostureMontage(AnimalData ? AnimalData->LieDownMontage.Get() : nullptr);
	
	GetWorldTimerManager().SetTimer(PostureTimer,
		FTimerDelegate::CreateUObject(this, &ABaseAnimal::SetPosture, EAnimalPosture::Resting),
		FMath::Max(Duration, 0.01f), false);
}

void ABaseAnimal::FallAsleep()
{
	if (Posture == EAnimalPosture::Resting)
	{
		SetPosture(EAnimalPosture::Sleeping);
	}
}

void ABaseAnimal::StartGettingUp()
{
	if (Posture == EAnimalPosture::Standing || Posture == EAnimalPosture::GettingUp) return;
	
	UAnimMontage* Montage = nullptr;
	if (AnimalData)
	{
		Montage = (Posture == EAnimalPosture::Sleeping)
			? AnimalData->GetUpFromSleepMontage.Get()
			: AnimalData->GetUpFromRestMontage.Get();
	}
	
	GetWorldTimerManager().ClearTimer(PostureTimer);
	SetPosture(EAnimalPosture::GettingUp);
	
	const float Duration = PlayPostureMontage(Montage);
	GetWorldTimerManager().SetTimer(PostureTimer,
		FTimerDelegate::CreateUObject(this, &ABaseAnimal::SetPosture, EAnimalPosture::Standing),
		FMath::Max(Duration, 0.01f), false);
}

void ABaseAnimal::HandleDeath(UHealthComponent* HealthComponent, AController* Contr)
{
	if (bIsDying) return;
	bIsDying = true;
	
	const FVector Velocity = GetCharacterMovement()->Velocity;
	
	GetWorldTimerManager().ClearTimer(PostureTimer);
	StopAnimMontage();
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* Brain = AIC->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("Dead"));
		}
	}
	
	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->Velocity = Velocity;
	Move->BrakingFrictionFactor = 0.0f;
	Move->BrakingDecelerationWalking = StumbleDeceleration;
	Move->bRunPhysicsWithNoController = true;
	
	StopAnimMontage();
	float Delay = StumbleTime;
	if (AnimalData && AnimalData->DeathMontage.Get())
	{
		PlayAnimMontage(AnimalData->DeathMontage.Get());
	}
	
	GetWorldTimerManager().SetTimer(DeathTimer, this, &ABaseAnimal::StartRagdoll, FMath::Max(Delay, 0.01f), false);
	
}

float ABaseAnimal::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const auto& PointEvent = static_cast<const FPointDamageEvent&>(DamageEvent);
		
		LastHitBone      = PointEvent.HitInfo.BoneName;
		LastHitLocation  = PointEvent.HitInfo.ImpactPoint;
		LastHitDirection = PointEvent.ShotDirection;
		
		if (const float* Mult = BoneDamageMultipliers.Find(PointEvent.HitInfo.BoneName))
		{
			DamageAmount *= *Mult;
		}
	}
	
	const float Actual = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	if (Actual > 0.0f && !HealthComp->IsDead())
	{
		AActor* Shooter = (EventInstigator && EventInstigator->GetPawn()) ? EventInstigator->GetPawn() : DamageCauser;
		
		UAISense_Damage::ReportDamageEvent(GetWorld(), this, Shooter, Actual, Shooter ? Shooter->GetActorLocation() : GetActorLocation(), GetActorLocation());
	}
	
	return Actual;
}

// Called when the game starts or when spawned
void ABaseAnimal::BeginPlay()
{
	Super::BeginPlay();
	
	HealthComp->OnDeath.AddUniqueDynamic(this, &ABaseAnimal::HandleDeath);
	HealthComp->OnHealthChanged.AddUniqueDynamic(this, &ABaseAnimal::HandleHealthChanged);
	
	SetRunning(false);
}

void ABaseAnimal::HandleHealthChanged(UHealthComponent* HealthComponent, float NewHealth, float Delta)
{
	if (Delta >= 0 || HealthComp->IsDead()) return;
	
	if (!IsStanding())
	{
		StartGettingUp();
	}
	
	bIsGrazing = false;
	SetRunning(true);
}

void ABaseAnimal::SetPosture(EAnimalPosture NewPosture)
{
	Posture = NewPosture;
}

float ABaseAnimal::PlayPostureMontage(UAnimMontage* Montage)
{
	return Montage ? PlayAnimMontage(Montage) : 0.0f;
}

void ABaseAnimal::StartRagdoll()
{
	const FVector CarryVelocity = GetCharacterMovement()->Velocity * DeathMomentumScale;
	
	DetachFromControllerPendingDestroy();
	
	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->StopMovementImmediately();
	Move->DisableMovement();
	Move->SetComponentTickEnabled(false);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	USkeletalMeshComponent* SkelMesh = GetMesh();
	SkelMesh->SetCollisionProfileName(TEXT("Ragdoll"));
	SkelMesh->SetAllBodiesSimulatePhysics(true);
	SkelMesh->SetSimulatePhysics(true);
	SkelMesh->WakeAllRigidBodies();
	SkelMesh->bBlendPhysics = true;
	SkelMesh->SetLinearDamping(RagdollLinearDamping);
	SkelMesh->SetAllPhysicsLinearVelocity(CarryVelocity);

	if (!LastHitDirection.IsNearlyZero())
	{
		const FName Bone = (LastHitBone != NAME_None) ? LastHitBone : AnimalData->FallbackBoneName;
		SkelMesh->AddImpulseAtLocation(LastHitDirection * BulletImpulse, LastHitLocation, Bone);
	}
}

