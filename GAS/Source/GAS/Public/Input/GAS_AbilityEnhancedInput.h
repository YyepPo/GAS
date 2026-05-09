// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "Data/AbilityInputInfo.h"
#include "GAS_AbilityEnhancedInput.generated.h"


class UAbilityInputInfo;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAS_API UGAS_AbilityEnhancedInput : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:

	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType, typename ConfirmFuncType,typename CancelFuncType>
	void BindAbilityAction(const UAbilityInputInfo* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc,ConfirmFuncType,CancelFuncType);
	
};

template <class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType, typename ConfirmFuncType,typename CancelFuncType>
void UGAS_AbilityEnhancedInput::BindAbilityAction(const UAbilityInputInfo* InputConfig, UserClass* Object,
	PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc,ConfirmFuncType ConfirmFunc,CancelFuncType CancelFunc)
{
	check(InputConfig);

	if (InputConfig->ConfirmAction && ConfirmFunc != nullptr)
	{
		BindAction(InputConfig->ConfirmAction, ETriggerEvent::Started, Object, ConfirmFunc);
	}

	if (InputConfig->CancelAction && CancelFunc != nullptr)
	{
		BindAction(InputConfig->CancelAction, ETriggerEvent::Started, Object, CancelFunc);
	}
	
	for (FAbilityInputStruct Input : InputConfig->AbilityInputList)
	{
		if (Input.InputAction != nullptr && Input.AbilityInputTag.IsValid())
		{
			if (PressedFunc != nullptr)
			{
				BindAction(Input.InputAction, ETriggerEvent::Started, Object, PressedFunc, Input.AbilityInputTag);
			}

			if (ReleasedFunc != nullptr)
			{
				BindAction(Input.InputAction,ETriggerEvent::Completed,Object, ReleasedFunc,Input.AbilityInputTag);
			}

			if (HeldFunc != nullptr)
			{
				BindAction(Input.InputAction,ETriggerEvent::Ongoing,Object, HeldFunc, Input.AbilityInputTag);
			}
		}
	}
	
	/*for (FAbilityInputStruct Input : InputConfig->AbilityInputList)
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
			
			//if (HeldFunc != nullptr)
			//{
			//	BindAction(Input.InputAction,ETriggerEvent::Triggered,Object,HeldFunc,Input.AbilityInputTag);
			//}
		}
	}*/
}
