#include "AbilityComponent/GAS_AbilitySystemComponent.h"
#include "GAS_GameplayTags.h"
#include "Abilities/GAS_AbilitySystemLibrary.h"
#include "Abilities/GAS_BaseAbility.h"
#include "Data/GAS_AbilityTypes.h"

// Sets default values for this component's properties
UGAS_AbilitySystemComponent::UGAS_AbilitySystemComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

}

// Called when the game starts
void UGAS_AbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UGAS_AbilitySystemComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UGAS_AbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& Abilities)
{
	for (const TSubclassOf<UGameplayAbility> AbilityClass : Abilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass,1);
		if (const UGAS_BaseAbility* Ability = Cast<UGAS_BaseAbility>(AbilitySpec.Ability))
		{
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(Ability->InputTag);
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(FGAS_GameplayTags::Get().Abilities_Status_Equipped);
			GiveAbility(AbilitySpec);
		}
	}
	bStartupAbilitiesGiven = true;
	AbilitiesGivenEvent.Broadcast();
}

void UGAS_AbilitySystemComponent::AddPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& PassiveAbilities)
{
	for (const TSubclassOf<UGameplayAbility> AbilityClass : PassiveAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass,1);
		GiveAbilityAndActivateOnce(AbilitySpec);
	}
}

FGameplayTag UGAS_AbilitySystemComponent::GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (FGameplayTag StatusTag : AbilitySpec.GetDynamicSpecSourceTags())
	{
		if (StatusTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Status"))))
		{
			return StatusTag;
		}
	}
	return FGameplayTag();
}

void UGAS_AbilitySystemComponent::AddCharacterAbilitiesAndActive(const TArray<TSubclassOf<UGameplayAbility>>& Abilities)
{
	for (const TSubclassOf<UGameplayAbility> AbilityClass : Abilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass,1);
		if (const UGAS_BaseAbility* Ability = Cast<UGAS_BaseAbility>(AbilitySpec.Ability))
		{
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(Ability->InputTag);
			GiveAbilityAndActivateOnce(AbilitySpec);
			
		}
	}
}

void UGAS_AbilitySystemComponent::ForEachAbility(const FForEachAbility& Delegate)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!Delegate.ExecuteIfBound(AbilitySpec))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to execute delegate in %hs"), __FUNCTION__);
		}
	}
}

void UGAS_AbilitySystemComponent::RemoveAllCharacterAbilities()
{
	TArray<FGameplayAbilitySpecHandle> Abilities;
	GetAllAbilities(Abilities);
	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : Abilities)
	{
		ClearAbility (AbilitySpecHandle);
	}
}

void UGAS_AbilitySystemComponent::RemoveCharacterAbility(const FGameplayAbilitySpecHandle& AbilitySpecHandle)
{
	if (AbilitySpecHandle.IsValid())
	{
		ClearAbility(AbilitySpecHandle);
	}
}

void UGAS_AbilitySystemComponent::OnAbilityInputPressed(FGameplayTag Tag)
{
	if (!Tag.IsValid())
	{
		return;
	}
	
	ABILITYLIST_SCOPE_LOCK();

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(Tag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			if (AbilitySpec.IsActive())
			{
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed,
					AbilitySpec.Handle,
					AbilitySpec.ActivationInfo.GetActivationPredictionKey());
			}
		}
	}
}

void UGAS_AbilitySystemComponent::OnAbilityInputReleased(FGameplayTag Tag)
{
	if (!Tag.IsValid()) return;
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(Tag) && AbilitySpec.IsActive())
		{
			AbilitySpecInputReleased(AbilitySpec);
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilitySpec.Handle, AbilitySpec.ActivationInfo.GetActivationPredictionKey());
		}
	}
}

void UGAS_AbilitySystemComponent::ActivateAbilityByTag(FGameplayTag AbilityTag)
{
	if(!AbilityTag.IsValid())
	{
		return;
	}

	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if(AbilitySpec.Ability->AbilityTags.HasTagExact(AbilityTag))
		{
			if(AbilitySpec.IsActive() == false)
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
			else
			{
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed,
					AbilitySpec.Handle,
					AbilitySpec.ActivationInfo.GetActivationPredictionKey());
			}
		}
	}
}

FGameplayTag UGAS_AbilitySystemComponent::GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	if (AbilitySpec.Ability)
	{
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags)
		{
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities"))))
			{
				return Tag;
			}
		}
	}
	return FGameplayTag();
}

FGameplayTag UGAS_AbilitySystemComponent::GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (FGameplayTag Tag : AbilitySpec.DynamicAbilityTags)
	{
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("InputTag"))))
		{
			return Tag;
		}
	}
	return FGameplayTag();
}
