// Fill out your copyright notice in the Description page of Project Settings.


#include "Animals/Wolf.h"

#include "Animals/WolfPackSubsystem.h"
#include "Components/HealthComponent.h"
#include "Datas/WolfData.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"

AWolf::AWolf()
{
}

const UWolfData* AWolf::GetWolfData() const
{
	return Cast<UWolfData>(AnimalData.Get());
}

bool AWolf::CanHowl() const
{
	const UWolfData* Data = GetWolfData();
	return Data && GetWorld()->GetTimeSeconds() - LastHowlTime >= Data->HowlCooldown;
}

float AWolf::Howl()
{
	MarkHowled();
	
	const UWolfData* Data = GetWolfData();
	return (Data && Data->HowlMontage) ? PlayAnimMontage(Data->HowlMontage) : 0.0f;
}

void AWolf::MarkHowled()
{
	LastHowlTime = GetWorld()->GetTimeSeconds();
}

int32 AWolf::PickAttack(float Distance) const
{
	const UWolfData* Data = GetWolfData();
	if (!Data) return INDEX_NONE;
	
	TArray<int32, TInlineAllocator<8>> Valid;
	float TotalWeight = 0.0f;
	
	for (int32 i = 0; i < Data->Attacks.Num(); i++)
	{
		const FWolfAttack& Attack = Data->Attacks[i];
		if (Attack.Weight <= 0.0f) continue;
		if (Distance >= 0.0f && (Distance < Attack.MinRange || Distance > Attack.MaxRange)) continue;

		Valid.Add(i);
		TotalWeight += Attack.Weight;
	}
	
	if (Valid.Num() == 0) return INDEX_NONE;
	
	float Roll = FMath::FRand() * TotalWeight;
	for (const int32 Index : Valid)
	{
		Roll -= Data->Attacks[Index].Weight;
		if (Roll <= 0.0f) return Index;
	}
	return Valid.Last();
}

float AWolf::StartAttack(AActor* Target, int32 AttackIndex)
{
	const UWolfData* Data = GetWolfData();
	if (!Data || !Target || bAttacking || HealthComp->IsDead()) return 0.0f;
	if (!Data->Attacks.IsValidIndex(AttackIndex)) return 0.0f;
	
	const FWolfAttack& Attack = Data->Attacks[AttackIndex];
	
	bAttacking = true;
	bHitThisAttack = false;
	CurrentAttack = AttackIndex;
	AttackTarget = Target;
	
	const FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	SetActorRotation(ToTarget.Rotation());
	
	if (Attack.LungeSpeed > 0.0f || Attack.LungeUpSpeed > 0.0f)
	{
		LaunchCharacter(ToTarget * Attack.LungeSpeed + FVector(0.f, 0.f, Attack.LungeUpSpeed), true, false);
	}
	
	float Duration = Attack.Montage ? PlayAnimMontage(Attack.Montage) : 0.0f;
	if (Duration <= 0.0f)
	{
		Duration = 0.8f;
	}
	
	FTimerManager& Timers = GetWorldTimerManager();
	if (Attack.HitTime > 0.0f)
	{
		Timers.SetTimer(AttackHitTimer, this, &AWolf::AttackHitCheck, Attack.HitTime, false);
	}
	Timers.SetTimer(AttackEndTimer, this, &AWolf::EndAttack, Duration, false);
	
	return Duration;
}

void AWolf::AttackHitCheck()
{
	const UWolfData* Data = GetWolfData();
	AActor* Target = AttackTarget.Get();
	if (!bAttacking || bHitThisAttack || !Data || !Target || HealthComp->IsDead()) return;
	if (!Data->Attacks.IsValidIndex(CurrentAttack)) return;
	
	const FWolfAttack& Attack = Data->Attacks[CurrentAttack];
	
	const FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	const float Reach = Data->AttackReach + GetSimpleCollisionRadius() + Target->GetSimpleCollisionRadius();
	
	if (ToTarget.SizeSquared2D() > FMath::Square(Reach)) return;
	if (FMath::Abs(ToTarget.Z) > 200.0f) return;
	if (FVector::DotProduct(GetActorForwardVector(), ToTarget.GetSafeNormal2D()) < 0.5f) return;
	
	if (const UHealthComponent* TargetHealth = Target->FindComponentByClass<UHealthComponent>())
	{
		if (TargetHealth->IsDead()) return;
	}
	
	FHitResult Block;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(WolfBite), false, this);
	Params.AddIgnoredActor(Target);
	TArray<AActor*> TargetAttached;
	Target->GetAttachedActors(TargetAttached);
	Params.AddIgnoredActors(TargetAttached);
	if (GetWorld()->LineTraceSingleByChannel(Block, GetActorLocation(), Target->GetActorLocation(), ECC_Visibility, Params))
	{
		return;
	}
	
	bHitThisAttack = true;
	UGameplayStatics::ApplyDamage(Target, Attack.Damage, GetController(), this, UDamageType::StaticClass());
	
	if (Attack.Knockback > 0.0f)
	{
		if (ACharacter* TargetChar = Cast<ACharacter>(Target))
		{
			const FVector Push = ToTarget.GetSafeNormal2D() * Attack.Knockback + FVector(0.f, 0.f, Attack.Knockback * 0.25f);
			TargetChar->LaunchCharacter(Push, true, false);
		}
	}
}

void AWolf::HandleDeath(UHealthComponent* HealthComponent, AController* Contr)
{
	EndAttack();
	
	if (UWorld* World = GetWorld())
	{
		if (UWolfPackSubsystem* Pack = World->GetSubsystem<UWolfPackSubsystem>())
		{
			Pack->ReleaseAttackToken(this);
			Pack->UnregisterWolf(this);
		}
	}
	
	Super::HandleDeath(HealthComponent, Contr);
}

void AWolf::BeginPlay()
{
	Super::BeginPlay();
	
	if (!GetWolfData())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: AnimalData is not a UWolfData asset"), *GetName());
	}
	
	ApplyRandomVariant();
	
	if (UWolfPackSubsystem* Pack = GetWorld()->GetSubsystem<UWolfPackSubsystem>())
	{
		Pack->RegisterWolf(this);
	}
}

void AWolf::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWolfPackSubsystem* Pack = GetWorld()->GetSubsystem<UWolfPackSubsystem>())
	{
		Pack->UnregisterWolf(this);
	}
	
	Super::EndPlay(EndPlayReason);
}

void AWolf::EndAttack()
{
	bAttacking = false;
	AttackTarget.Reset();
	
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AttackHitTimer);
		World->GetTimerManager().ClearTimer(AttackEndTimer);
	}
}

void AWolf::ApplyRandomVariant()
{
	const UWolfData* Data = GetWolfData();
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!Data || !MeshComp || Data->Variants.Num() == 0) return;
	
	const int32 Index = Data->Variants.IsValidIndex(ForcedVariant)
	? ForcedVariant
	: FMath::RandRange(0, Data->Variants.Num() - 1);
	
	const FWolfVariant& Variant = Data->Variants[Index];

	if (Variant.BodyMaterial)
	{
		MeshComp->SetMaterialByName(TEXT("default"), Variant.BodyMaterial);
	}
	if (Variant.FurMaterial)
	{
		MeshComp->SetMaterialByName(TEXT("Wolf_Fur"), Variant.FurMaterial);
	}
}
