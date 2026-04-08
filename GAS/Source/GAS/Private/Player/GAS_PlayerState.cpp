#include "Public/Player/GAS_PlayerState.h"
#include "AbilitySystemComponent.h"

AGAS_PlayerState::AGAS_PlayerState()
{
	// Create ability system component and mark it as replicated
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("Ability System Component"));
	AbilitySystemComponent->SetIsReplicated(true);

	// increase the NetUpdateFrequency of your PlayerState.
	// It defaults to a very low value on the PlayerState and can cause delays or perceived lag before changes to things like Attributes and GameplayTags happen on the clients.
	// Be sure to enable Adaptive Network Update Frequency, Fortnite uses it.
	SetNetUpdateFrequency(20.f);
}

UAbilitySystemComponent* AGAS_PlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
