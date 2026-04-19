#include "GAS_AbilitySystemGlobals.h"
#include "Data/GAS_AbilityTypes.h"

FGameplayEffectContext* GAS_AbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FGAS_GameplayEffectContext();
}
