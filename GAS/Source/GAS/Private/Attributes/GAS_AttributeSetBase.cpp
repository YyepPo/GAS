#include "Public/Attributes/GAS_AttributeSetBase.h"

UGAS_AttributeSetBase::UGAS_AttributeSetBase()
{
	
}

void UGAS_AttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	
}

void UGAS_AttributeSetBase::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
}

void UGAS_AttributeSetBase::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
}
