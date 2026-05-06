#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GAS_ListenForStackChange.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStackCountChangedEvent, const FActiveGameplayEffectHandle&, ActiveGameplayEffectHandle,float, NewCount,float, OldCount);

USTRUCT(BlueprintType)
struct FStackThresholdConfig
{
	GENERATED_BODY()

	// The max stack count that triggers the effect
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	int32 StackThreshold = 5;

	// The GE to apply when threshold is reached
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> EffectToApply;
};

UCLASS()
class GAS_API UGAS_ListenForStackChange : public UGameplayAbility
{
	GENERATED_BODY()

	UGAS_ListenForStackChange();

public:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UPROPERTY(Blueprintassignable)
	FOnStackCountChangedEvent OnStackCountChangedEvent;
	
private:

	void OnGameplayEffectAdded(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& GameplayEffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle);
	void OnGameplayEffectRemoved( const FActiveGameplayEffect& ActiveGameplayEffect);
	void OnStackCountChanged(FActiveGameplayEffectHandle GameplayEffectHandle, int32 NewStackCount, int32 OldStackCount);
	
	FGameplayTag WatchedStackTag;
	
	// Key   = asset tag on the GE (e.g. GameplayEffect.HitStack)
	// Value = what to do when the threshold is hit
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "Stack Listening", meta = (AllowPrivateAccess = "true"))
	TMap<FGameplayTag, FStackThresholdConfig> StackConfigs;
};
