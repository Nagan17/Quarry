// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BloodComponent.generated.h"

class UBloodData;
class UDecalComponent;
class UMaterialInterface;
class USkeletalMeshComponent;

/**
 * Owns all blood on a creature: hit spray (Niagara), ground splash, bleeding trail and death pool.
 * Fed by the owner's TakeDamage; knows nothing about animals, so it can go on the wolf or the player too.
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class QUARRY_API UBloodComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBloodComponent();

	/** Call from the owner's BeginPlay. PoolBone is where the death pool is traced from (e.g. pelvis). */
	void Init(const UBloodData* InData, USkeletalMeshComponent* InMesh, FName InPoolBone);

	/** A bullet hit. ShotDir is the bullet's travel direction. */
	void OnPointHit(const FVector& HitLocation, const FVector& ShotDir, FName Bone, float Damage);

	/** Owner died: keep dripping briefly, then leave a pool once the ragdoll settles. */
	void OnDeath();

	UFUNCTION(BlueprintPure, Category="Blood")
	bool IsBleeding() const { return BleedIntensity > 0.f; }

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	/** Niagara user parameter names on NS_BloodHit. */
	static const FName SprayDirectionParam;
	static const FName IntensityParam;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void SpawnSpray(const FVector& Location, const FVector& Direction, FName Bone, float Intensity) const;
	bool FindExitPoint(const FVector& Entry, const FVector& Dir, FHitResult& OutHit) const;
	bool TraceGround(const FVector& From, FHitResult& OutHit) const;
	void SpawnDecal(UMaterialInterface* Mat, const FVector& Location, const FRotator& Rotation, float Size);
	void SpawnDeathPool();
	float GetCurrentBleed() const;

	static UMaterialInterface* PickRandom(const TArray<TObjectPtr<UMaterialInterface>>& Mats);
	static FRotator RandomDecalRotation(const FVector& Normal);
	static FRotator AlignedDecalRotation(const FVector& Normal, const FVector& AlongDir, float ExtraRoll);

	UPROPERTY()
	TObjectPtr<const UBloodData> Data;

	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> Mesh;

	TArray<TWeakObjectPtr<UDecalComponent>> Decals;

	FName WoundBone = NAME_None;
	FName PoolBone = NAME_None;
	float BleedIntensity = 0.f;
	float BleedTimeLeft = 0.f;
	float DistSinceDrop = 0.f;
	float TimeSinceDrop = 0.f;
	FVector LastLocation = FVector::ZeroVector;
	bool bDead = false;
	FTimerHandle PoolTimer;
};
