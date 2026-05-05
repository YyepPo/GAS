#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GAS_AbilitySystemComponent.generated.h"


DECLARE_MULTICAST_DELEGATE_OneParam(FDeactivatePassiveAbility, const FGameplayTag& /*AbilityTag*/);
DECLARE_MULTICAST_DELEGATE(FAbilitiesGiven);
DECLARE_MULTICAST_DELEGATE_FourParams(FAbilityEquipped, const FGameplayTag& /*AbilityTag*/, const FGameplayTag& /*Status*/, const FGameplayTag& /*Slot*/, const FGameplayTag& /*PrevSlot*/);
DECLARE_DELEGATE_OneParam(FForEachAbility, const FGameplayAbilitySpec&);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAS_API UGAS_AbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UGAS_AbilitySystemComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	// Add abilities but dont active them
	UFUNCTION(BlueprintCallable)
	void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& Abilities);
	
	void AddPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& PassiveAbilities);

	static FGameplayTag GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec);

	void ForEachAbility(const FForEachAbility& Delegate);
	
	// Abilities will be automatically activated
	UFUNCTION(BlueprintCallable)
	void AddCharacterAbilitiesAndActive(const TArray<TSubclassOf<UGameplayAbility>>& Abilities);
	
	UFUNCTION(BlueprintCallable)
	void RemoveAllCharacterAbilities();
	
	UFUNCTION(BlueprintCallable)
	void RemoveCharacterAbility(const FGameplayAbilitySpecHandle& AbilitySpecHandle);
	
	UFUNCTION(BlueprintCallable)
	void OnAbilityInputPressed(FGameplayTag Tag);
	
	UFUNCTION(BlueprintCallable)
	void OnAbilityInputReleased(FGameplayTag Tag);
	
	UFUNCTION(BlueprintCallable)
	void OnAbilityInputHeld(FGameplayTag Tag);
	
	UFUNCTION(BlueprintCallable)
	void ActivateAbilityByTag(FGameplayTag AbilityTag);

	UFUNCTION(BlueprintCallable)
	void SpendAttributePoint(const FGameplayTag& AttributeTag);
	
	FGameplayTag GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	FGameplayTag GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);

	bool bStartupAbilitiesGiven = false;

	// Mirrors what the async task does internally
	void OnActiveGameplayEffectAdded(UAbilitySystemComponent* ASC,
		const FGameplayEffectSpec& Spec,
		FActiveGameplayEffectHandle Handle);

	void OnAnyGameplayEffectRemoved(const FActiveGameplayEffect& Effect);

	void OnHitStackChanged(FActiveGameplayEffectHandle Handle,
		int32 NewCount, int32 OldCount);

	FGameplayTag WatchedStackTag;
	
	///
	///Delegates
	///

	FAbilitiesGiven AbilitiesGivenEvent;
	FAbilityEquipped AbilityEquippedEvent;
	FForEachAbility ForeachAbility;
};
