// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GAS_BaseAbility.h"
#include "Interaction/IInteractableTarget.h"
#include "GAS_InteractionAbility.generated.h"

struct FInteractionOption;
/**
 * 
 */
UCLASS()
class GAS_API UGAS_InteractionAbility : public UGAS_BaseAbility
{

	GENERATED_BODY()

public:
	
	UGAS_InteractionAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
 	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	UFUNCTION(BlueprintCallable)
	void UpdateInteraction(const TArray<FInteractionOption>& InInteractionOptions);
	UFUNCTION(BlueprintCallable)
	bool TriggerInteraction();
	
private:

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Ability|Radius Scan", meta=(AllowPrivateAccess="true", Units="s"))
	float ScanRate = 0.1;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Ability|Radius Scan", meta=(AllowPrivateAccess="true"))
	float ScanRadius = 300.f;

	AActor* GetActorFromInteractableTarget(TScriptInterface<IIInteractableTarget> InteractableTarget);
	
	UPROPERTY()
	TArray<FInteractionOption> CurrentOptions;
};
