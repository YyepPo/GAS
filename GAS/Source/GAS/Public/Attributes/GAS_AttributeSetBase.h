#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GAS_AttributeSetBase.generated.h"

// Uses macros from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class GAS_API UGAS_AttributeSetBase : public UAttributeSet
{
	GENERATED_BODY()

public:

	UGAS_AttributeSetBase();


	/** Attribute set overrides */ 

	/**	This function is meant to enforce things like "Health = Clamp(Health, 0, MaxHealth)" and NOT things like "trigger this extra thing if damage is applied, etc". */ 
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue);
	/** Called just after any modification happens to an attribute. */
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	/** Called just after a GameplayEffect is executed to modify the base value of an attribute. No more changes can be made.
	Note this is only called during an 'execute'. E.g., a modification to the 'base value' of an attribute. It is not called during an application of a GameplayEffect, such as a 5 second +10 movement speed buff. */
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	/** Attribute set overrides END */
	
};
