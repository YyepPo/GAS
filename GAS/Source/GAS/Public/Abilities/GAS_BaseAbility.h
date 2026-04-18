#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GAS_BaseAbility.generated.h"

UCLASS()
class GAS_API UGAS_BaseAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag;
	
protected:
	
	float GetManaCost(float Level = 1.f) const;
	float GetCooldown(float Level = 1.f) const;
	
};
