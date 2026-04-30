// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "AttributeInformation.generated.h"

USTRUCT(BlueprintType)
struct FAuraAttributeInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AttributeTag = FGameplayTag();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText Name = FText();
	
	UPROPERTY(BlueprintReadOnly)
	float Value = 0.f;
};


UCLASS()
class GAS_API UAttributeInformation : public UDataAsset
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable)
	FAuraAttributeInfo FindAttributeInfoForTag(const FGameplayTag& AttributeTag) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FAuraAttributeInfo> AttributeInformation;
};
