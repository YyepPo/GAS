// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/GAS_PassiveAbility.h"

#include "AbilityComponent/GAS_AbilitySystemComponent.h"

void UGAS_PassiveAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                          const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                          const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	UGAS_AbilitySystemComponent* AbilitySystemComponent = Cast<UGAS_AbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	if (AbilitySystemComponent)
	{
		//AbilitySystemComponent->DeactivatePassiveAbility.AddUObject(this,&UGAS_PassiveAbility::ReceiveDeactivate);
	}
}

void UGAS_PassiveAbility::ReceiveDeactivate(const FGameplayTag& AbilityTag)
{
	if (AbilityTags.HasTagExact(AbilityTag))
	{
		EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,true);
	}
}
