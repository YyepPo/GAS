#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "AbilityInputConfig.generated.h"

class UInputAction;

USTRUCT(Blueprintable)
struct FAbilityInputConfig
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	UInputAction* InputAction;
	
	UPROPERTY(EditAnywhere)
	FGameplayTag AbilityInputTag;
	
};

UCLASS()
class GAS_API UAbilityInputConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere)
	FAbilityInputConfig AbilityInputConfig;
};
