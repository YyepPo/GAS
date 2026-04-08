#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
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

	/** Current Health of the player.
	* This variable is replicated.
	* Capped by MaxHealth
	*/
	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "Health")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UGAS_AttributeSetBase, Health)

	/** Max Health Amount. Capes Health amount
	 * Can be updated during runtime.
	 * This variable is replicated.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_MaxHealth, BlueprintReadOnly, Category = "Health")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UGAS_AttributeSetBase, MaxHealth)

	/** Increase health amount every second.
	 * Can be updated during runtime
	 * This variable is replicated.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_HealthRegenRate, BlueprintReadOnly, Category = "Health")
	FGameplayAttributeData HealthRegenRate;
	ATTRIBUTE_ACCESSORS(UGAS_AttributeSetBase, HealthRegenRate)
	
	/** Current Mana of the player.
	* Used to execute abilities.
	* This variable is replicated.
	* Capped by MaxMana
	*/
	UPROPERTY(ReplicatedUsing = OnRep_Mana, BlueprintReadOnly, Category = "Mana")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UGAS_AttributeSetBase, Mana)

	/** Max Mana Amount. Capes Mana amount
	 * Can be updated during runtime.
	 * This variable is replicated.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_MaxMana, BlueprintReadOnly, Category = "Mana")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UGAS_AttributeSetBase, MaxMana)

	/** Current Stamina of the player.
	* Used to execute abilities.
	* This variable is replicated.
	* Capped by MaxStamina
	*/
	UPROPERTY(ReplicatedUsing = OnRep_Stamina, BlueprintReadOnly, Category = "Mana")
	FGameplayAttributeData Stamina ;
	ATTRIBUTE_ACCESSORS(UGAS_AttributeSetBase, Stamina)

	/** Max Mana Amount. Capes Mana amount
	 * Can be updated during runtime.
	 * This variable is replicated.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_MaxStamina, BlueprintReadOnly, Category = "Mana")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UGAS_AttributeSetBase, MaxStamina)

	/** Increase stamina amount every second.
	 * Can be updated during runtime
	 * This variable is replicated.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_StaminaRegenRate, BlueprintReadOnly, Category = "Health")
	FGameplayAttributeData StaminaRegenRate;
	ATTRIBUTE_ACCESSORS(UGAS_AttributeSetBase, StaminaRegenRate)
	
};
