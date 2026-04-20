#include "GAS_AbilitySystemGlobals.h"
#include "Data/GAS_AbilityTypes.h"

FGameplayEffectContext* UGAS_AbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FGAS_GameplayEffectContext;
}
