// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractionQuery.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Interaction/IInteractableTarget.h"
#include "AT_WaitForInteractableTarget.generated.h"

template <typename InterfaceType> class TScriptInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractableObjectsChangedEvent, const TArray<FInteractionOption>&, InteractableOptions);

UCLASS()
class GAS_API UAT_WaitForInteractableTarget : public UAbilityTask
{
	GENERATED_BODY()

	UAT_WaitForInteractableTarget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
public: 

	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;
	
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAT_WaitForInteractableTarget* WaitForInteractableTarget(UGameplayAbility* OwningAbility,
		FInteractionQueryResult InteractionQuery,
		FCollisionProfileName TraceProfile,
		FGameplayAbilityTargetingLocationInfo StartLocation,
		float InteractionScanRange = 100,
		float InteractionScanRate = 0.100,
		bool bShowDebug = false);

	UPROPERTY(BlueprintAssignable)
	FInteractableObjectsChangedEvent InteractableObjectsChanged;

private:

	FTimerHandle DetectionTimerHandle;
	UFUNCTION()
	void OnDetectionTimerHandleFinished();

	void AimWithPlayerController(AActor* Actor, const FCollisionQueryParams& Params, const FVector& TraceStart, float MaxRange, FVector& OutTraceEnd);
	bool ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection, FVector AbilityCenter, float AbilityRange, FVector& ClippedPosition);
	void LineTrace(FHitResult& HitResult, UWorld* World, const FVector& TraceStart, const FVector& TraceEnd, FName ProfileName, const FCollisionQueryParams& Params);
	void UpdateInteractableOptions(const FInteractionQueryResult& InteractQuery, const TArray<TScriptInterface<IIInteractableTarget>>& InteractableTargets);
	bool bTraceAffectsAimPitch;

	TArray<FInteractionOption> CurrentOptions;
	
	UPROPERTY()
	FInteractionQueryResult InteractionQuery;

	FCollisionProfileName TraceProfile;
	FGameplayAbilityTargetingLocationInfo StartLocation;
	float InteractionScanRange = 100;
	float InteractionScanRate = 0.100;
	bool bShowDebug = false;
	
};
