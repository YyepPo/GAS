#pragma once

#include "CoreMinimal.h"
#include "Abilities/GAS_BaseAbility.h"
#include "GAS_MeleeAttack_Ability.generated.h"

class UGAS_AbilityTask_MeleeTrace;

UCLASS()
class GAS_API UGAS_MeleeAttack_Ability : public UGAS_BaseAbility
{
	GENERATED_BODY()

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

private:
	UFUNCTION()
	void OnHitWindowOpen(FGameplayEventData Payload);

	UFUNCTION()
	void OnHitWindowClose(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnTraceHit(const FHitResult& HitResult);

protected:
	// Set these on the ability CDO / per blueprint subclass
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	FGameplayTag DamageType;

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float BaseDamage = 20.f;

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float DeathImpulseMagnitude = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float KnockbackForceMagnitude = 500.f;

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float KnockbackChance = 20.f; // percent

	UPROPERTY(EditDefaultsOnly, Category="Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category="Trace")
	FName TipSocket  = "WeaponTip";

	UPROPERTY(EditDefaultsOnly, Category="Trace")
	FName RootSocket = "WeaponRoot";

	UPROPERTY(EditDefaultsOnly, Category="Trace")
	float SweepRadius = 30.f;

private:
	UPROPERTY()
	TObjectPtr<UGAS_AbilityTask_MeleeTrace> MeleeTraceTask;
};
