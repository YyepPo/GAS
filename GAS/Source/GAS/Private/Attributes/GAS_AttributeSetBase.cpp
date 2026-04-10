#include "Public/Attributes/GAS_AttributeSetBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

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

void UGAS_AttributeSetBase::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// Clamp Health
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(),0,GetMaxHealth()));
	}

	// Clamp Mana
	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(),0,GetMaxMana()));
	}

	// Clamp Stamina
	if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(),0,GetMaxStamina()));
	}
	
	// Handle incoming damage
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float LocalIncomingDamage = GetIncomingDamage();
		SetIncomingDamage(0);
		HandleIncomingDamage(LocalIncomingDamage);
	}

}

void UGAS_AttributeSetBase::HandleIncomingDamage(const float& IncomingDamageAmount)
{
	UE_LOG(LogTemp,Warning,TEXT("Attribute Set: Damage: Incoming damage %f"),IncomingDamageAmount);

	const float NewHealth = GetHealth() - IncomingDamageAmount;
	SetHealth(FMath::Clamp(NewHealth,0,GetMaxHealth()));
}

void UGAS_AttributeSetBase::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
}

void UGAS_AttributeSetBase::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_AttributeSetBase, Health, OldHealth);
}

void UGAS_AttributeSetBase::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_AttributeSetBase, MaxHealth, OldMaxHealth);
}

void UGAS_AttributeSetBase::OnRep_HealthRegenRate(const FGameplayAttributeData& OldHealthRegenRate) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_AttributeSetBase, HealthRegenRate, OldHealthRegenRate);
}

void UGAS_AttributeSetBase::OnRep_Mana(const FGameplayAttributeData& OldMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_AttributeSetBase, Mana, OldMana);
}

void UGAS_AttributeSetBase::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_AttributeSetBase, MaxMana, OldMaxMana);
}

void UGAS_AttributeSetBase::OnRep_Stamina(const FGameplayAttributeData& OldStamina) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_AttributeSetBase, Stamina, OldStamina);
}

void UGAS_AttributeSetBase::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_AttributeSetBase, MaxStamina, OldMaxStamina);
}

void UGAS_AttributeSetBase::OnRep_StaminaRegenRate(const FGameplayAttributeData& OldStaminaRegenRate) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_AttributeSetBase, StaminaRegenRate, OldStaminaRegenRate);
}

void UGAS_AttributeSetBase::OnRep_Strength(const FGameplayAttributeData& OldStrength) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_AttributeSetBase, Strength, OldStrength);
}

void UGAS_AttributeSetBase::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_AttributeSetBase, Intelligence, OldIntelligence);
}

void UGAS_AttributeSetBase::OnRep_Stealth(const FGameplayAttributeData& OldStealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_AttributeSetBase, Stealth, OldStealth);
}
