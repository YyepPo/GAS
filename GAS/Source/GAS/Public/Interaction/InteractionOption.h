#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "Abilities/GameplayAbility.h"
#include "InteractionOption.generated.h"

// this struct lives inside the item, defines which ability the item is going to give, display text, the widget that will be shown

class IIInteractableTarget;
class UGameplayAbility;

USTRUCT(BlueprintType)
struct FInteractionOption
{
	GENERATED_BODY()
	
public:

	/* The interactable target **/
	UPROPERTY()
	TScriptInterface<IIInteractableTarget> InteractableTarget;

	/* Simple text that item might return **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Text;

	/* Description that item might return **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/* Ability that will be granted to the avatar/controller that interacts with this item **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayAbility> AbilityToGrant;

	/** The ability system on the target that can be used for the TargetInteractionHandle and sending the event, if needed. */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UAbilitySystemComponent> TargetAbilitySystem = nullptr;

	/** The ability spec to activate on the object for this option. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayAbilitySpecHandle TargetInteractionAbilityHandle;

	FORCEINLINE bool operator==(const FInteractionOption& Other) const
	{
		return InteractableTarget == Other.InteractableTarget &&
			AbilityToGrant == Other.AbilityToGrant &&
			TargetAbilitySystem == Other.TargetAbilitySystem &&
			TargetInteractionAbilityHandle == Other.TargetInteractionAbilityHandle &
			Text.IdenticalTo(Other.Text) &&
			Description.IdenticalTo(Other.Description);
	}

	FORCEINLINE bool operator!=(const FInteractionOption& Other) const
	{
		return !operator==(Other);
	}

	FORCEINLINE bool operator<(const FInteractionOption& Other) const
	{
		return InteractableTarget.GetInterface() < Other.InteractableTarget.GetInterface();
	}
};
