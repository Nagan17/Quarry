// Fill out your copyright notice in the Description page of Project Settings.


#include "Animals/BaseAnimal.h"

#include <Components/HealthComponent.h>

#include "Animals/AnimalAIController.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"

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
	
}

float ABaseAnimal::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const auto& PointEvent = static_cast<const FPointDamageEvent&>(DamageEvent);
		if (const float* Mult = BoneDamageMultipliers.Find(PointEvent.HitInfo.BoneName))
		{
			DamageAmount *= *Mult;
		}
	}
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

// Called when the game starts or when spawned
void ABaseAnimal::BeginPlay()
{
	Super::BeginPlay();
	
	HealthComp->OnDeath.AddUniqueDynamic(this, &ABaseAnimal::HandleDeath);
	
	SetRunning(false);
}

void ABaseAnimal::SetPosture(EAnimalPosture NewPosture)
{
	Posture = NewPosture;
}

float ABaseAnimal::PlayPostureMontage(UAnimMontage* Montage)
{
	return Montage ? PlayAnimMontage(Montage) : 0.0f;
}

