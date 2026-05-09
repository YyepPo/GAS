#pragma once

#include "CoreMinimal.h"
#include "Abilities/GAS_BaseAbility.h"
//#include "GameplayAbilityTargetTypes.h"
#include "GameplayTagContainer.h"
#include "GAS_IceRain.generated.h"

class AGAS_GroundTargetActor;
class AGAS_IceRainSpike;
class UAbilityTask_WaitDelay;
class UAbilityTask_WaitTargetData;
class UGameplayEffect;

UCLASS()
class GAS_API UGAS_IceRain : public UGAS_BaseAbility
{
	GENERATED_BODY()

public:
	UGAS_IceRain();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void OnTargetDataReady(const FGameplayAbilityTargetDataHandle& TargetData);

	UFUNCTION()
	void OnTargetDataCancelled(const FGameplayAbilityTargetDataHandle& TargetData);

	UFUNCTION()
	void OnRainDurationFinished();

	void BeginTargetingPhase();
	void BeginIceRain(const FVector& TargetLocation);
	void SpawnIceSpike();
	FVector GetRandomSpikeSpawnLocation() const;

	UPROPERTY(EditDefaultsOnly, Category = "Ice Rain|Targeting")
	TSubclassOf<AGAS_GroundTargetActor> TargetActorClass;

	UPROPERTY(EditDefaultsOnly, Category = "Ice Rain|Targeting", meta = (ClampMin = "1.0", Units = "cm"))
	float TargetRadius = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Ice Rain|Spawning")
	TSubclassOf<AGAS_IceRainSpike> IceRainSpikeClass;

	UPROPERTY(EditDefaultsOnly, Category = "Ice Rain|Spawning", meta = (ClampMin = "0.1", Units = "s"))
	float RainDuration = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "Ice Rain|Spawning", meta = (ClampMin = "0.02", Units = "s"))
	float SpawnInterval = 0.25f;

	UPROPERTY(EditDefaultsOnly, Category = "Ice Rain|Spawning", meta = (ClampMin = "0.0", Units = "cm"))
	float SpawnHeight = 1200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Ice Rain|Spawning", meta = (ClampMin = "0.0", Units = "cm/s"))
	float SpikeFallSpeed = 2200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Ice Rain|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Ice Rain|Damage")
	TSubclassOf<UGameplayEffect> StunEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Ice Rain|Damage", meta = (ClampMin = "0.0"))
	float BaseDamage = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Ice Rain|Damage", meta = (Categories = "Damage"))
	FGameplayTag DamageType;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitTargetData> TargetDataTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitDelay> RainDurationTask;

	FTimerHandle SpawnTimerHandle;
	FVector LockedTargetLocation = FVector::ZeroVector;
};
