// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractionOption.h"
#include "UObject/Interface.h"
#include "IInteractableTarget.generated.h"

struct FInteractionQueryResult;
struct FGameplayTag;
struct FGameplayEventData;

class FInteractionOptionBuilder
{
public:
	FInteractionOptionBuilder(TScriptInterface<IIInteractableTarget> InterfaceTargetScope, TArray<FInteractionOption>& InteractOptions)
		: Scope(InterfaceTargetScope)
		, Options(InteractOptions)
	{
	}

	void AddInteractionOption(const FInteractionOption& Option)
	{
		FInteractionOption& OptionEntry = Options.Add_GetRef(Option);
		OptionEntry.InteractableTarget = Scope;
	}

private:
	TScriptInterface<IIInteractableTarget> Scope;
	TArray<FInteractionOption>& Options;
};


// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UIInteractableTarget : public UInterface
{
	GENERATED_BODY()
};
/**
 * 
 */
class GAS_API IIInteractableTarget
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual void GatherInteractionOptions(const FInteractionQueryResult& InteractionQueryResult, FInteractionOptionBuilder& InteractionOptionBuilder) = 0;

	virtual void CustomizeInteractionEventData(const FGameplayTag& InteractionEventTag, FGameplayEventData& InOutEventData) { }

};
