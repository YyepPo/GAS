#include "Abilities/GAS_BaseAbility.h"
#include "Attributes/GAS_AttributeSetBase.h"

float UGAS_BaseAbility::GetManaCost(float Level) const
{
	float ManaCost = 0.f;
	
	if (const UGameplayEffect* ManaCostEffect = GetCostGameplayEffect())
	{
		for (FGameplayModifierInfo Modifier : ManaCostEffect->Modifiers)
		{
			if (Modifier.Attribute == UGAS_AttributeSetBase::GetManaAttribute())
			{
				Modifier.ModifierMagnitude.GetStaticMagnitudeIfPossible(Level,ManaCost);
			}
		}
	}
	

	return ManaCost;
}

float UGAS_BaseAbility::GetCooldown(float Level) const 
{
	float Cooldown = 0.f;
	
	if (const UGameplayEffect* CooldownEffect = GetCostGameplayEffect())
	{
		CooldownEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(Level,Cooldown);
	}
	
	return Cooldown;
}
