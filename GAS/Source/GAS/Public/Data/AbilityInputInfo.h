// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputAction.h"
#include "Engine/DataAsset.h"
#include "AbilityInputInfo.generated.h"

USTRUCT(Blueprintable)
struct FAbilityInputStruct
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	UInputAction* InputAction;
	
	UPROPERTY(EditAnywhere)
	FGameplayTag AbilityInputTag;
	
};


UCLASS()
class GAS_API UAbilityInputInfo : public UDataAsset
{
	GENERATED_BODY()

	public:

	UPROPERTY(EditAnywhere)
	TArray<FAbilityInputStruct> AbilityInputList;
};
