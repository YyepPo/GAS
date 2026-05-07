#include "Abilities/GAS_ListenForStackChange.h"
#include "AbilitySystemComponent.h"

UGAS_ListenForStackChange::UGAS_ListenForStackChange()
{
	
}

void UGAS_ListenForStackChange::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (AbilitySystemComponent == nullptr)
	{
		EndAbility(Handle,ActorInfo,ActivationInfo,true,false);
		return;
	}
		
	AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this,&UGAS_ListenForStackChange::OnGameplayEffectAdded);
}

void UGAS_ListenForStackChange::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGAS_ListenForStackChange::OnGameplayEffectAdded(UAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayEffectSpec& GameplayEffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle)
{
	FGameplayTagContainer AssetTags;
	GameplayEffectSpec.GetAllAssetTags(AssetTags);

	for (auto& [Tag, Config] : StackConfigs)
	{
		if (AssetTags.HasTagExact(Tag))
		{
			FOnActiveGameplayEffectStackChange* StackDelegate =
				AbilitySystemComponent->OnGameplayEffectStackChangeDelegate(ActiveGameplayEffectHandle);

			if (StackDelegate)
			{
				StackDelegate->AddUObject(this,&UGAS_ListenForStackChange::OnStackCountChanged);
			}

			break;
		}
	}
}

void UGAS_ListenForStackChange::OnGameplayEffectRemoved(const FActiveGameplayEffect& ActiveGameplayEffect)
{
	if (!ActiveGameplayEffect.Spec.Def) return;

	FGameplayTagContainer AssetTags;
	ActiveGameplayEffect.Spec.GetAllAssetTags(AssetTags);
	OnStackGameplayEffectRemoved.Broadcast(AssetTags, ActiveGameplayEffect.Spec.GetStackCount());
}

void UGAS_ListenForStackChange::OnStackCountChanged(FActiveGameplayEffectHandle GameplayEffectHandle, int32 NewStackCount,
	int32 OldStackCount)
{
	OnStackCountChangedEvent.Broadcast(GameplayEffectHandle,NewStackCount,OldStackCount);
}
