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

USTRUCT()
struct FEffectProperties
{
	GENERATED_BODY()

	FEffectProperties(){}

	FGameplayEffectContextHandle EffectContextHandle;

	UPROPERTY()
	UAbilitySystemComponent* SourceASC = nullptr;

	UPROPERTY()
	AActor* SourceAvatarActor = nullptr;

	UPROPERTY()
	AController* SourceController = nullptr;

	UPROPERTY()
	ACharacter* SourceCharacter = nullptr;

	UPROPERTY()
	UAbilitySystemComponent* TargetASC = nullptr;

	UPROPERTY()
	AActor* TargetAvatarActor = nullptr;

	UPROPERTY()
	AController* TargetController = nullptr;

	UPROPERTY()
	ACharacter* TargetCharacter = nullptr;
};

// typedef is specific to the FGameplayAttribute() signature, but TStaticFunPtr is generic to any signature chosen
//typedef TBaseStaticDelegateInstance<FGameplayAttribute(), FDefaultDelegateUserPolicy>::FFuncPtr FAttributeFuncPtr;
template<class T>
using TStaticFuncPtr = typename TBaseStaticDelegateInstance<T, FDefaultDelegateUserPolicy>::FFuncPtr;


UCLASS()
class GAS_API UGAS_AttributeSetBase : public UAttributeSet
{
	GENERATED_BODY()

public:

	UGAS_AttributeSetBase();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;	
	
	/** Attribute set overrides */ 

	/**	This function is meant to enforce things like "Health = Clamp(Health, 0, MaxHealth)" and NOT things like "trigger this extra thing if damage is applied, etc". */ 
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	/** Called just after any modification happens to an attribute. */
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	/** Called just after a GameplayEffect is executed to modify the base value of an attribute. No more changes can be made.
	Note this is only called during an 'execute'. E.g., a modification to the 'base value' of an attribute. It is not called during an application of a GameplayEffect, such as a 5 second +10 movement speed buff. */
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	/** Attribute set overrides END */

	TMap<FGameplayTag, TStaticFuncPtr<FGameplayAttribute()>> TagsToAttributes;

	
	/** 
	 * Meta attributes
	 */
	
	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UGAS_AttributeSetBase, IncomingDamage)

	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes")
	FGameplayAttributeData IncomingXp;
	ATTRIBUTE_ACCESSORS(UGAS_AttributeSetBase, IncomingXp)
	
	/** 
	 * Vital Attributes 
	 * 
	 */

	UPROPERTY(BlueprintReadOnly, Category = "Primary")
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(UGAS_AttributeSetBase, Armor)

	UPROPERTY(BlueprintReadOnly, Category = "Secondary Attributes")
	FGameplayAttributeData CriticalHitChance;
	ATTRIBUTE_ACCESSORS(UGAS_AttributeSetBase, CriticalHitChance);

	UPROPERTY(BlueprintReadOnly, Category = "Secondary Attributes")
	FGameplayAttributeData CriticalHitDamage;
	ATTRIBUTE_ACCESSORS(UGAS_AttributeSetBase, CriticalHitDamage);
	
	/** Current Health of the player.
	* This variable is replicated.
	* Capped by MaxHealth
	*/
	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "Primary")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UGAS_AttributeSetBase, Health)

	/** Max Health Amount. Capes Health amount
	 * Can be updated during runtime.
	 * This variable is replicated.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_MaxHealth, BlueprintReadOnly, Category = "Primary")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UGAS_AttributeSetBase, MaxHealth)

	/** Increase health amount every second.
	 * Can be updated during runtime
	 * This variable is replicated.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_HealthRegenRate, BlueprintReadOnly, Category = "Primary")
	FGameplayAttributeData HealthRegenRate;
	ATTRIBUTE_ACCESSORS(UGAS_AttributeSetBase, HealthRegenRate)
	
	/** Current Mana of the player.
	* Used to execute abilities.
	* This variable is replicated.
	* Capped by MaxMana
	*/
	UPROPERTY(ReplicatedUsing = OnRep_Mana, BlueprintReadOnly, Category = "Primary")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UGAS_AttributeSetBase, Mana)

	/** Max Mana Amount. Capes Mana amount
	 * Can be updated during runtime.
	 * This variable is replicated.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_MaxMana, BlueprintReadOnly, Category = "Primary")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UGAS_AttributeSetBase, MaxMana)

	UPROPERTY(ReplicatedUsing = OnRep_ManaRegenRate,BlueprintReadOnly, Category = "Primary")
	FGameplayAttributeData ManaRegenRate;
	ATTRIBUTE_ACCESSORS(UGAS_AttributeSetBase,ManaRegenRate)

	/** Current Stamina of the player.
	* Used to execute abilities.
	* This variable is replicated.
	* Capped by MaxStamina
	*/
	UPROPERTY(ReplicatedUsing = OnRep_Stamina, BlueprintReadOnly, Category = "Primary")
	FGameplayAttributeData Stamina ;
	ATTRIBUTE_ACCESSORS(UGAS_AttributeSetBase, Stamina)

	/** Max Mana Amount. Capes Mana amount
	 * Can be updated during runtime.
	 * This variable is replicated.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_MaxStamina, BlueprintReadOnly, Category = "Primary")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UGAS_AttributeSetBase, MaxStamina)

	/** Increase stamina amount every second.
	 * Can be updated during runtime
	 * This variable is replicated.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_StaminaRegenRate, BlueprintReadOnly, Category = "Primary")
	FGameplayAttributeData StaminaRegenRate;
	ATTRIBUTE_ACCESSORS(UGAS_AttributeSetBase, StaminaRegenRate)
	
	
	/** 
	 * Physical Attribute Set  
	 */
	
	/* Modifies physical damage, stamina cost, stamina regen, **/
	UPROPERTY(ReplicatedUsing = OnRep_Strength ,BlueprintReadOnly,Category = "Phyiscal")
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS(UGAS_AttributeSetBase,Strength)
	
	/* Modifies magical/spell/ability damage, mana costs, cooldown reduction, **/
	UPROPERTY(ReplicatedUsing = OnRep_Intelligence ,BlueprintReadOnly,Category = "Phyiscal")
	FGameplayAttributeData Intelligence;
	ATTRIBUTE_ACCESSORS(UGAS_AttributeSetBase,Intelligence)
	
	/* Modifies magical/spell/ability damage, mana costs, cooldown reduction, **/
	UPROPERTY(ReplicatedUsing = OnRep_Vigor ,BlueprintReadOnly,Category = "Phyiscal")
	FGameplayAttributeData Vigor;
	ATTRIBUTE_ACCESSORS(UGAS_AttributeSetBase,Vigor)

private:

	void HandleIncomingDamage(const FEffectProperties& Props);
	void DisplayDamageNumberText(const FEffectProperties& Props,float IncomingDamageAmount);
	void SendXPEvent(const FEffectProperties& Props);
	void HandleIncomingXP(const FEffectProperties& Props); 
	void SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const; 
	
protected:
	
	/* 
	 * On Reps
	 * **/
	
	// ====================== Primary / Vital Attributes ======================
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;

	UFUNCTION()
	void OnRep_HealthRegenRate(const FGameplayAttributeData& OldHealthRegenRate) const;

	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldMana) const;

	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const;

	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldStamina) const;

	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina) const;

	UFUNCTION()
	void OnRep_StaminaRegenRate(const FGameplayAttributeData& OldStaminaRegenRate) const;

	UFUNCTION()
	void OnRep_ManaRegenRate(const FGameplayAttributeData& OldManaRegenRate) const;

	// ====================== Physical / Mental Attributes ======================
	UFUNCTION()
	void OnRep_Strength(const FGameplayAttributeData& OldStrength) const;

	UFUNCTION()
	void OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const;

	UFUNCTION()
	void OnRep_Vigor(const FGameplayAttributeData& OldVigor) const;
};