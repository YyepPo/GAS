// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "Data/AbilityInputConfig.h"
#include "GAS_AbilityEnhancedInput.generated.h"


class UAbilityInputConfig;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAS_API UGAS_AbilityEnhancedInput : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:

	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
	void BindAbilityAction(const UAbilityInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc);
	
};

template <class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
void UGAS_AbilityEnhancedInput::BindAbilityAction(const UAbilityInputConfig* InputConfig, UserClass* Object,
	PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc)
{
	check(InputConfig);

	for (FAbilityInputConfig Input : InputConfig->AbilityInputConfig)
	{
		if (Input.InputAction != nullptr && Input.AbilityInputTag.IsValid())
		{
			if (PressedFunc != nullptr)
			{
				BindAction(Input.InputAction, ETriggerEvent::Started, Object, PressedFunc, Input.AbilityInputTag);
			}
			
			if (ReleasedFunc != nullptr)
			{
				BindAction(Input.InputAction, ETriggerEvent::Completed,Object,ReleasedFunc,Input.AbilityInputTag);
			}
			
			if (HeldFunc != nullptr)
			{
				BindAction(Input.InputAction,ETriggerEvent::Triggered,Object,HeldFunc,Input.AbilityInputTag);
			}
		}
	}
}
