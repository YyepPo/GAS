#include "Public/Attributes/GAS_AttributeSetBase.h"

#include "Net/UnrealNetwork.h"

UGAS_AttributeSetBase::UGAS_AttributeSetBase()
{
	
}

void UGAS_AttributeSetBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	
	DOREPLIFETIME_CONDITION_NOTIFY(UGAS_AttributeSetBase, Health,           COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGAS_AttributeSetBase, MaxHealth,        COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGAS_AttributeSetBase, HealthRegenRate,  COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UGAS_AttributeSetBase, Mana,             COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGAS_AttributeSetBase, MaxMana,          COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UGAS_AttributeSetBase, Stamina,          COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGAS_AttributeSetBase, MaxStamina,       COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGAS_AttributeSetBase, StaminaRegenRate, COND_None, REPNOTIFY_Always);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UGAS_AttributeSetBase, Strength,         COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGAS_AttributeSetBase, Intelligence,     COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGAS_AttributeSetBase, Stealth,          COND_None, REPNOTIFY_Always);
}

void UGAS_AttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	// Clamp Health
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue,0,GetMaxHealth());
	}
	// Clamp Max Health, make sure MaxHealth stays above 1
	if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue,1);
	}
	// Clamp Mana
	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue,0,GetMaxMana());	
	}
	// Clamp Max Mana, make sure MaxMana stays above 1
	if (Attribute == GetMaxManaAttribute())
	{
		NewValue = FMath::Max(NewValue,1);
	}
	// Clamp Stamina
	if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue,0,GetMaxStamina());
	}
	// Clamp Max Stamina, make sure MaxStamina stays above 1
	if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Max(NewValue,1);
	}
	
}

void UGAS_AttributeSetBase::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
}

void UGAS_AttributeSetBase::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
}

void UGAS_AttributeSetBase::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
}

void UGAS_AttributeSetBase::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
}

void UGAS_AttributeSetBase::OnRep_HealthRegenRate(const FGameplayAttributeData& OldHealthRegenRate) const
{
}

void UGAS_AttributeSetBase::OnRep_Mana(const FGameplayAttributeData& OldMana) const
{
}

void UGAS_AttributeSetBase::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{
}

void UGAS_AttributeSetBase::OnRep_Stamina(const FGameplayAttributeData& OldStamina) const
{
}

void UGAS_AttributeSetBase::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina) const
{
}

void UGAS_AttributeSetBase::OnRep_StaminaRegenRate(const FGameplayAttributeData& OldStaminaRegenRate) const
{
}

void UGAS_AttributeSetBase::OnRep_Strength(const FGameplayAttributeData& OldStrength) const
{
}

void UGAS_AttributeSetBase::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const
{
}

void UGAS_AttributeSetBase::OnRep_Stealth(const FGameplayAttributeData& OldStealth) const
{
}
