// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/BloodComponent.h"
#include "Components/DecalComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"
#include "Datas/BloodData.h"

const FName UBloodComponent::SprayDirectionParam(TEXT("User.SprayDirection"));
const FName UBloodComponent::IntensityParam(TEXT("User.Intensity"));

UBloodComponent::UBloodComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; // only ticks while bleeding
	PrimaryComponentTick.TickInterval = 0.1f;           // drops don't need per-frame checks
}

void UBloodComponent::Init(const UBloodData* InData, USkeletalMeshComponent* InMesh, FName InPoolBone)
{
	Data = InData;
	Mesh = InMesh;
	PoolBone = InPoolBone;

	if (!Data)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: no BloodData set, blood effects disabled"), *GetNameSafe(GetOwner()));
	}
}

void UBloodComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PoolTimer);
	}
	Super::EndPlay(EndPlayReason);
}

// ---------------------------------------------------------------------------
// Hit
// ---------------------------------------------------------------------------

void UBloodComponent::OnPointHit(const FVector& HitLocation, const FVector& ShotDir, FName Bone, float Damage)
{
	if (!Data) return;

	const FVector Dir = ShotDir.GetSafeNormal();
	const float Intensity = FMath::GetMappedRangeValueClamped(
		FVector2D(Data->DamageForMinIntensity, Data->DamageForMaxIntensity),
		FVector2D(Data->MinIntensity, Data->MaxIntensity),
		Damage);

	SpawnSpray(HitLocation, Dir.IsNearlyZero() ? FVector::UpVector : -Dir, Bone, Intensity);

	FVector SplashOrigin = HitLocation;
	if (Data->bSpawnExitSpray && !Dir.IsNearlyZero())
	{
		FHitResult ExitHit;
		if (FindExitPoint(HitLocation, Dir, ExitHit))
		{
			SpawnSpray(ExitHit.ImpactPoint, Dir, ExitHit.BoneName, Intensity * Data->ExitIntensityScale);
			SplashOrigin = ExitHit.ImpactPoint;
		}
	}

	if (UMaterialInterface* SplashMat = PickRandom(Data->SplashDecals))
	{
		FHitResult Ground;
		if (TraceGround(SplashOrigin, Ground))
		{
			const FVector Along = Dir.IsNearlyZero() ? GetOwner()->GetActorForwardVector() : Dir;
			SpawnDecal(SplashMat, Ground.ImpactPoint,
				AlignedDecalRotation(Ground.ImpactNormal, Along, Data->SplashRollOffset),
				Data->SplashDecalSize * FMath::Lerp(0.8f, 1.2f, FMath::FRand()));
		}
	}

	if (bDead) return;

	WoundBone = Bone;
	BleedIntensity = FMath::Clamp(BleedIntensity + Damage * Data->DamageToBleed, 0.f, 1.f);
	BleedTimeLeft = Data->BleedDuration;
	LastLocation = GetOwner()->GetActorLocation();
	DistSinceDrop = TimeSinceDrop = 0.f;
	SetComponentTickEnabled(true);
}

void UBloodComponent::SpawnSpray(const FVector& Location, const FVector& Direction, FName Bone, float Intensity) const
{
	if (!Data || !Data->HitFX || !Mesh) return;

	UNiagaraComponent* FX = UNiagaraFunctionLibrary::SpawnSystemAttached(
		Data->HitFX, Mesh, Bone, Location, Direction.Rotation(),
		EAttachLocation::KeepWorldPosition, true, false);

	if (!FX) return;

	FX->SetVariableVec3(SprayDirectionParam, Direction);
	FX->SetVariableFloat(IntensityParam, Intensity);
	FX->Activate(true);
}

bool UBloodComponent::FindExitPoint(const FVector& Entry, const FVector& Dir, FHitResult& OutHit) const
{
	if (!Mesh || !Data) return false;

	const FVector Start = Entry + Dir * Data->MaxBodyThickness;
	const FVector End = Entry + Dir * 1.f;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(BloodExitTrace), false);
	return Mesh->LineTraceComponent(OutHit, Start, End, Params);
}

// ---------------------------------------------------------------------------
// Bleeding trail
// ---------------------------------------------------------------------------

float UBloodComponent::GetCurrentBleed() const
{
	if (!Data) return 0.f;
	// The trail thins out as the wound "closes"
	return BleedIntensity * FMath::Clamp(BleedTimeLeft / FMath::Max(Data->BleedDuration, 0.01f), 0.f, 1.f);
}

void UBloodComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	BleedTimeLeft -= DeltaTime;
	const float Bleed = GetCurrentBleed();
	if (Bleed <= 0.02f || !Data || Data->TrailDecals.IsEmpty())
	{
		BleedIntensity = 0.f;
		SetComponentTickEnabled(false);
		return;
	}

	const FVector Loc = GetOwner()->GetActorLocation();
	DistSinceDrop += FVector::Dist2D(Loc, LastLocation);
	LastLocation = Loc;
	TimeSinceDrop += DeltaTime;

	// Distance-based while moving (a sprinting deer leaves a readable trail), time-based while still
	const float Spacing  = Data->DropSpacing / FMath::Max(Bleed, 0.15f);
	const float IdleTime = Data->IdleDropInterval / FMath::Max(Bleed, 0.15f);
	if (DistSinceDrop < Spacing && TimeSinceDrop < IdleTime) return;

	DistSinceDrop = TimeSinceDrop = 0.f;

	// Drip from under the wound, not the capsule centre
	FVector From = (Mesh && WoundBone != NAME_None) ? Mesh->GetBoneLocation(WoundBone) : Loc;
	From += FVector(FMath::FRandRange(-1.f, 1.f), FMath::FRandRange(-1.f, 1.f), 0.f) * Data->DropScatter;

	FHitResult Ground;
	if (TraceGround(From, Ground))
	{
		const float Size = FMath::FRandRange(Data->TrailDecalSize.X, Data->TrailDecalSize.Y)
			* FMath::Lerp(0.6f, 1.f, Bleed);
		SpawnDecal(PickRandom(Data->TrailDecals), Ground.ImpactPoint, RandomDecalRotation(Ground.ImpactNormal), Size);
	}
}

// ---------------------------------------------------------------------------
// Death
// ---------------------------------------------------------------------------

void UBloodComponent::OnDeath()
{
	if (bDead) return;
	bDead = true;

	// The trail keeps dripping through the stumble; the pool replaces it once the body settles.
	if (Data && Data->PoolDecal)
	{
		GetWorld()->GetTimerManager().SetTimer(PoolTimer, this,
			&UBloodComponent::SpawnDeathPool, FMath::Max(Data->PoolDelay, 0.01f), false);
	}
	else
	{
		SetComponentTickEnabled(false);
	}
}

void UBloodComponent::SpawnDeathPool()
{
	BleedIntensity = 0.f;
	SetComponentTickEnabled(false);

	if (!Mesh || !Data || !Data->PoolDecal) return;

	const FVector From = (PoolBone != NAME_None) ? Mesh->GetBoneLocation(PoolBone) : Mesh->GetComponentLocation();
	FHitResult Ground;
	if (!TraceGround(From + FVector(0.f, 0.f, 50.f), Ground)) return;
	
	UDecalComponent* Pool = SpawnDecal(Data->PoolDecal, Ground.ImpactPoint, RandomDecalRotation(Ground.ImpactNormal), Data->PoolSize);
	if (!Pool) return;
	
	Pool->SetWorldScale3D(FVector(1.f, Data->PoolStartScale, Data->PoolStartScale));
	
	PoolDecalComp = Pool;
	PoolGrowStartTime = GetWorld()->GetTimeSeconds();
	GetWorld()->GetTimerManager().SetTimer(PoolGrowTimer, this, &UBloodComponent::UpdatePoolGrowth, 0.033f, true);
	
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

bool UBloodComponent::TraceGround(const FVector& From, FHitResult& OutHit) const
{
	FCollisionQueryParams Params(SCENE_QUERY_STAT(BloodGroundTrace), false, GetOwner());
	return GetWorld()->LineTraceSingleByChannel(OutHit, From, From - FVector(0.f, 0.f, 400.f),
		ECC_Visibility, Params);
}

UDecalComponent* UBloodComponent::SpawnDecal(UMaterialInterface* Mat, const FVector& Location, const FRotator& Rotation, float Size)
{
	if (!Mat || !Data) nullptr;

	UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(
		this, Mat, FVector(10.f, Size, Size), Location, Rotation);
	
	if (!Decal) nullptr;

	Decal->SetFadeScreenSize(0.002f); // cull tiny far-away drops
	Decal->SetFadeOut(Data->DecalLifetime, Data->DecalFadeTime, false); // also sets the lifespan

	// Budget: remove the oldest
	Decals.RemoveAll([](const TWeakObjectPtr<UDecalComponent>& D) { return !D.IsValid(); });
	Decals.Add(Decal);
	while (Decals.Num() > Data->MaxDecals)
	{
		if (Decals[0].IsValid())
		{
			Decals[0]->DestroyComponent();
		}
		Decals.RemoveAt(0);
	}
	
	return Decal;
}

void UBloodComponent::UpdatePoolGrowth()
{
	UDecalComponent* Pool = PoolDecalComp.Get();
	if (!Pool || !Data)
	{
		GetWorld()->GetTimerManager().ClearTimer(PoolGrowTimer);
		return;
	}
	
	const float Elasped = GetWorld()->GetTimeSeconds() - PoolGrowStartTime;
	const float Alpha = FMath::Clamp(Elasped/Data->PoolGrowTime, 0.0f, 1.0f);
	
	const float Scale = FMath::InterpEaseOut(Data->PoolStartScale, 1.0f, Alpha, 2.0f);
	Pool->SetWorldScale3D(FVector(1.0f, Scale, Scale));
	
	if (Alpha >= 1.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(PoolGrowTimer);
	}
}

UMaterialInterface* UBloodComponent::PickRandom(const TArray<TObjectPtr<UMaterialInterface>>& Mats)
{
	return Mats.IsEmpty() ? nullptr : Mats[FMath::RandRange(0, Mats.Num() - 1)].Get();
}

FRotator UBloodComponent::RandomDecalRotation(const FVector& Normal)
{
	// Decals project along their +X, so aim into the surface; random roll for variety
	FRotator Rot = (-Normal).Rotation();
	Rot.Roll = FMath::FRandRange(0.f, 360.f);
	return Rot;
}

FRotator UBloodComponent::AlignedDecalRotation(const FVector& Normal, const FVector& AlongDir, float ExtraRoll)
{
	const FVector X = -Normal;
	FVector Y = FVector::VectorPlaneProject(AlongDir, Normal).GetSafeNormal();
	if (Y.IsNearlyZero())
	{
		return RandomDecalRotation(Normal); // shot straight down: no meaningful direction
	}

	FRotator Rot = FRotationMatrix::MakeFromXY(X, Y).Rotator();
	Rot.Roll += ExtraRoll;
	return Rot;
}
