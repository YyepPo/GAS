#pragma once

#include "Coreminimal.h"
#include "AbilitySystemGlobals.h"
#inclide "GAS_AbilitySystemGlobals.generated.h"

UCLASS()
class GAS_API GAS_AbilitySystemGlobals : public UAbilitySystemGlobals
{
	
	GENERATED_BODY()
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
	
};
