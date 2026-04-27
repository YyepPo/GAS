#pragma once

#include "CoreMinimal.h"
#include "Abilities/GAS_BaseAbility.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS_GlacialCharge.generated.h"

class ACharacter;
class AActor;
class AGAS_GlacialChargeIceBlock;
class UAnimMontage;
class UAbilityTask_ApplyRootMotionConstantForce;
class UAbilityTask_WaitDelay;
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

	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable)
	void ApplyDamageEffect(AActor* Actor);
	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable)
	void OnChargeStarts();
	
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
	void StartChargeExecution();
	void StartNextDashSegment();
	void StartTrailSpawning();
	void SpawnNextIceBlock();
	void StartChargeHitSweep();
	void UpdateChargeHitSweep();
	void HandleChargeSweepHit(AActor* HitActor, const FVector& SweepOrigin, const FVector& DashForward, float TraceAngleDegrees);
	bool ResolveIceBlockSpawnTransform(const FTransform& IntendedTransform, FTransform& OutSpawnTransform) const;
	void ReleaseChargeControlNearSplineEnd();
	void StartSplineCorrection();
	void UpdateSplineCorrection();
	void SnapCharacterToSplineDistance(float DistanceAlongSpline, bool bForceSnap);
	void ConfigureOwnerTrailCollision(bool bEnableIgnore);
	bool IsValidChargeTarget(AActor* CandidateActor) const;
	float GetActorFeetZ(const AActor* Actor) const;
	float GetIceBlockWorldHalfHeight(const FTransform& BlockTransform) const;
	bool TraceGroundSample(ACharacter* Character, const FVector& SamplePoint, FHitResult& OutHit) const;
	FVector ComputeSideKnockbackDirection(const AActor* HitActor, const FVector& DashForward, float TraceAngleDegrees);
	FVector GetDashDirection(const ACharacter* Character) const;
	FVector SampleSurfaceNormalAtDistance(float DistanceAlongSpline) const;
	FTransform MakeIceBlockTransform(float DistanceAlongSpline) const;
	void RestoreMovementMode();
	void CleanupChargeState();

	UFUNCTION()
	void OnDashSegmentFinished();

	UFUNCTION()
	void OnMontageTaskFinished();

	UFUNCTION()
	void OnChargeStartupDelayFinished();

	UPROPERTY(EditDefaultsOnly, Category = "Charge")
	float DashDuration = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge")
	float DashForce = 900.f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge")
	TObjectPtr<UAnimMontage> ChargeMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Charge")
	float ChargeStartupDelay = 0.46f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Collision Sweep")
	float SweepArcDegrees = 180.f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Collision Sweep")
	float SweepAngleStepDegrees = 30.f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Collision Sweep")
	float SweepTraceRange = 180.f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Collision Sweep")
	float SweepTraceInterval = 0.03f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Collision Sweep")
	float SweepTraceForwardOffset = 40.f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Collision Sweep")
	float SweepTraceHeightOffset = 45.f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Collision Sweep")
	float EnemyKnockbackForce = 850.f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Collision Sweep")
	float EnemyForwardKnockbackBlend = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Collision Sweep")
	float EnemyKnockbackUpForce = 120.f;

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
	float IceGroundTraceStartHeight = 220.f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Trail")
	float IceGroundTraceDepth = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Trail")
	float EnemyFootClearance = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Trail")
	TSubclassOf<AGAS_GlacialChargeIceBlock> IceBlockClass;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Spline Follow")
	float SplineCorrectionInterval = 0.016f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Spline Follow")
	float EndSplineReleaseDistance = 75.f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge|Debug")
	bool bDrawDebug = false;

	UPROPERTY(Transient)
	TObjectPtr<USplineComponent> DashSpline;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_ApplyRootMotionConstantForce> ActiveRootMotionTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> ActiveMontageTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitDelay> ActiveChargeDelayTask;

	FTimerHandle IceSpawnTimerHandle;
	FTimerHandle ChargeHitSweepTimerHandle;
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
	FVector CachedDashDirection = FVector::ForwardVector;
	TSet<TWeakObjectPtr<AActor>> AffectedChargeTargets;
	EMovementMode SavedMovementMode = MOVE_Walking;
	uint8 SavedCustomMovementMode = 0;
	bool bHasSavedMovementMode = false;
	bool bHasCleanedUp = false;
	bool bHasReleasedNearEndControl = false;
	bool bUseRightSideForCenterHit = true;
	bool bDashInProgress = false;
};
