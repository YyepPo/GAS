#pragma once

#include "CoreMinimal.h"
#include "Abilities/GAS_BaseAbility.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS_GlacialCharge.generated.h"

class ACharacter;
class AGAS_GlacialChargeIceBlock;
class UAnimMontage;
class UAbilityTask_ApplyRootMotionConstantForce;
class UAbilityTask_PlayMontageAndWait;
class UCapsuleComponent;
class USplineComponent;

USTRUCT()
struct FGlacialChargeDashSegment
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Direction = FVector::ZeroVector;

	UPROPERTY()
	float Distance = 0.f;

	UPROPERTY()
	float Duration = 0.f;

	UPROPERTY()
	float Strength = 0.f;
};

UCLASS()
class GAS_API UGAS_GlacialCharge : public UGAS_BaseAbility
{
	GENERATED_BODY()

public:
	UGAS_GlacialCharge();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	void CreateOrResetDashSpline(ACharacter* Character);
	bool BuildDashSpline(ACharacter* Character, const FVector& DashDirection);
	void BuildDashSegments();
	void BuildIceBlockTransforms();
	void StartNextDashSegment();
	void StartTrailSpawning();
	void SpawnNextIceBlock();
	void StartSplineCorrection();
	void UpdateSplineCorrection();
	void SnapCharacterToSplineDistance(float DistanceAlongSpline, bool bForceSnap);
	void ConfigureOwnerTrailCollision(bool bEnableIgnore);
	bool TraceGroundSample(ACharacter* Character, const FVector& SamplePoint, FHitResult& OutHit) const;
	FVector GetDashDirection(const ACharacter* Character) const;
	FVector SampleSurfaceNormalAtDistance(float DistanceAlongSpline) const;
	FTransform MakeIceBlockTransform(float DistanceAlongSpline) const;
	void RestoreMovementMode();
	void CleanupChargeState();

	UFUNCTION()
	void OnDashSegmentFinished();

	UFUNCTION()
	void OnMontageTaskFinished();

	UPROPERTY(EditDefaultsOnly, Category = "Charge")
	float DashDuration = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge")
	float DashForce = 900.f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge")
	TObjectPtr<UAnimMontage> ChargeMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Path")
	int32 TraceCount = 10;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Path")
	float TraceSpacing = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Path")
	float GroundTraceStartHeight = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Path")
	float GroundTraceDepth = 450.f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Path")
	float MinClimbableHeight = 50.f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Path")
	float MaxClimbableHeight = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Path")
	float SplineTension = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Path")
	float DashSegmentLength = 50.f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Trail")
	float IceSpawnInterval = 0.08f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Trail")
	float BlockSpacing = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Trail")
	FVector IceBlockDimensions = FVector(100.f, 10.f, 10.f);

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Trail")
	float TrailSurfaceOffset = 2.f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Trail")
	float BlockSpawnLeadDistance = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Trail")
	TSubclassOf<AGAS_GlacialChargeIceBlock> IceBlockClass;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Spline Follow")
	float SplineCorrectionInterval = 0.016f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Debug")
	bool bDrawDebug = false;

	UPROPERTY(Transient)
	TObjectPtr<USplineComponent> DashSpline;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_ApplyRootMotionConstantForce> ActiveRootMotionTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> ActiveMontageTask;

	FTimerHandle IceSpawnTimerHandle;
	FTimerHandle SplineCorrectionTimerHandle;

	TArray<FGlacialChargeDashSegment> DashSegments;
	TArray<FVector> DashSplineNormals;
	TArray<FTransform> PendingIceBlockTransforms;
	TArray<TWeakObjectPtr<AGAS_GlacialChargeIceBlock>> SpawnedIceBlocks;

	TWeakObjectPtr<ACharacter> CachedCharacter;
	TWeakObjectPtr<UCapsuleComponent> CachedCapsuleComponent;

	int32 CurrentDashSegmentIndex = INDEX_NONE;
	int32 NextIceBlockIndex = 0;
	float ActiveDashDistance = 0.f;
	float DashSplineVerticalOffset = 0.f;
	float EffectiveIceSpawnInterval = 0.f;
	float DashStartTimeSeconds = 0.f;
	EMovementMode SavedMovementMode = MOVE_Walking;
	uint8 SavedCustomMovementMode = 0;
	bool bHasSavedMovementMode = false;
	bool bHasCleanedUp = false;
	bool bDashInProgress = false;
};
