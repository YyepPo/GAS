// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AT_GrantNearbyInteraction.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UAT_GrantNearbyInteraction : public UAbilityTask
{
	GENERATED_BODY()

public:

	UFUNCTION(blueprintCallable)
	static UAT_GrantNearbyInteraction* CreateGrantNearbyInteraction(UGameplayAbility* OwningAbility,float ScanRadius,float ScanRate);

	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;
private:
	
	float ScanRadius = 300.f;
	float ScanRate = 0.1f;

	FTimerHandle ScanTimerHandle;
	UFUNCTION()
	void OnScanTimerHandleFinished();

	TMap<FObjectKey, FGameplayAbilitySpecHandle> InteractionAbilityCache;
};
