// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "GAS_AbilitySystemGlobals.generated.h"
UCLASS()
class GAS_API UGAS_AbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

public:

	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
	
};
