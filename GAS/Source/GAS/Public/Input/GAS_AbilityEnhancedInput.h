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

	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
	void BindAbilityAction(const UAbilityInputInfo* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc);
	
};

template <class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
void UGAS_AbilityEnhancedInput::BindAbilityAction(const UAbilityInputInfo* InputConfig, UserClass* Object,
	PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc)
{
	check(InputConfig);


	// Add this
	UE_LOG(LogTemp, Warning, TEXT("bbbbbbb AbilityInputList count: %d"), InputConfig->AbilityInputList.Num());

	for (FAbilityInputStruct Input : InputConfig->AbilityInputList)
	{
		// Add this
		UE_LOG(LogTemp, Warning, TEXT("Checking Input - Action: %s | Tag: %s"),
			Input.InputAction ? *Input.InputAction->GetName() : TEXT("NULL"),
			*Input.AbilityInputTag.ToString());

		if (Input.InputAction != nullptr && Input.AbilityInputTag.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("bbbbbbb Binding action: %s"), *Input.InputAction->GetName());
            
			if (PressedFunc != nullptr)
			{
				BindAction(Input.InputAction, ETriggerEvent::Started, Object, PressedFunc, Input.AbilityInputTag);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("bbbbbbb SKIPPED - InputAction is null or Tag is invalid"));
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
