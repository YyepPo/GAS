#include "Interaction/AT_WaitForInteractableTarget.h"

#include "AbilitySystemComponent.h"
#include "Interaction/IInteractableTarget.h"
#include "Interaction/InteractionQuery.h"

UAT_WaitForInteractableTarget::UAT_WaitForInteractableTarget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	
}

UAT_WaitForInteractableTarget* UAT_WaitForInteractableTarget::WaitForInteractableTarget(UGameplayAbility* OwningAbility,
                                                                                        FInteractionQueryResult InteractionQuery, FCollisionProfileName TraceProfile,
                                                                                        FGameplayAbilityTargetingLocationInfo StartLocation, float InteractionScanRange, float InteractionScanRate,
                                                                                        bool bShowDebug)
{
	UAT_WaitForInteractableTarget* Task = NewAbilityTask<UAT_WaitForInteractableTarget>(OwningAbility);
	Task->InteractionQuery = InteractionQuery;
	Task->TraceProfile = TraceProfile;
	Task->StartLocation = StartLocation;
	Task->InteractionScanRange = InteractionScanRange;
	Task->InteractionScanRate = InteractionScanRate;
	Task->bShowDebug = bShowDebug;
	return Task;	
}

void UAT_WaitForInteractableTarget::Activate()
{
	SetWaitingOnAvatar();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(DetectionTimerHandle,this,&UAT_WaitForInteractableTarget::OnDetectionTimerHandleFinished,InteractionScanRate,true);
	}
}

void UAT_WaitForInteractableTarget::OnDestroy(bool bInOwnerFinished)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DetectionTimerHandle);
	}
	
	Super::OnDestroy(bInOwnerFinished);
}



void UAT_WaitForInteractableTarget::OnDetectionTimerHandleFinished()
{
	AActor* AvatarActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
	if (!AvatarActor)
	{
		return;
	}

	UWorld* World = GetWorld();

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(AvatarActor);
	
	FCollisionQueryParams Params;
	Params.AddIgnoredActors(ActorsToIgnore);

	FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation();
	FVector TraceEnd;
	AimWithPlayerController(AvatarActor, Params, TraceStart, InteractionScanRange, OUT TraceEnd);

	FHitResult OutHitResult;
	LineTrace(OutHitResult, World, TraceStart, TraceEnd, TraceProfile.Name, Params);

	TArray<TScriptInterface<IIInteractableTarget>> InteractableTargets;

	TScriptInterface<IIInteractableTarget> InteractableTarget(OutHitResult.GetActor());
	if (InteractableTarget)
	{
		InteractableTargets.Add(InteractableTarget);
	}

	TScriptInterface<IIInteractableTarget> InteractableTargett(OutHitResult.GetComponent());
	if (InteractableTargett)
	{
		InteractableTargets.Add(InteractableTargett);
	}
	
	UpdateInteractableOptions(InteractionQuery, InteractableTargets);


	if (bShowDebug)
	{
		FColor DebugColor = OutHitResult.bBlockingHit ? FColor::Red : FColor::Green;
		if (OutHitResult.bBlockingHit)
		{
			DrawDebugLine(World, TraceStart, OutHitResult.Location, DebugColor, false, InteractionScanRate);
			DrawDebugSphere(World, OutHitResult.Location, 5, 16, DebugColor, false, InteractionScanRate);
		}
		else
		{
			DrawDebugLine(World, TraceStart, TraceEnd, DebugColor, false, InteractionScanRate);
		}
	}

}

void UAT_WaitForInteractableTarget::AimWithPlayerController(AActor* Actor, const FCollisionQueryParams& Params,
	const FVector& TraceStart, float MaxRange, FVector& OutTraceEnd)
{
	if (!Ability) // Server and launching client only
	{
		return;
	}

	//@TODO: Bots?
	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	check(PC);

	FVector ViewStart;
	FRotator ViewRot;
	PC->GetPlayerViewPoint(ViewStart, ViewRot);

	const FVector ViewDir = ViewRot.Vector();
	FVector ViewEnd = ViewStart + (ViewDir * MaxRange);

	ClipCameraRayToAbilityRange(ViewStart, ViewDir, TraceStart, MaxRange, ViewEnd);

	FHitResult HitResult;
	LineTrace(HitResult, Actor->GetWorld(), ViewStart, ViewEnd, TraceProfile.Name, Params);

	const bool bUseTraceResult = HitResult.bBlockingHit && (FVector::DistSquared(TraceStart, HitResult.Location) <= (MaxRange * MaxRange));

	const FVector AdjustedEnd = (bUseTraceResult) ? HitResult.Location : ViewEnd;

	FVector AdjustedAimDir = (AdjustedEnd - TraceStart).GetSafeNormal();
	if (AdjustedAimDir.IsZero())
	{
		AdjustedAimDir = ViewDir;
	}

	if (!bTraceAffectsAimPitch && bUseTraceResult)
	{
		FVector OriginalAimDir = (ViewEnd - TraceStart).GetSafeNormal();

		if (!OriginalAimDir.IsZero())
		{
			// Convert to angles and use original pitch
			const FRotator OriginalAimRot = OriginalAimDir.Rotation();

			FRotator AdjustedAimRot = AdjustedAimDir.Rotation();
			AdjustedAimRot.Pitch = OriginalAimRot.Pitch;

			AdjustedAimDir = AdjustedAimRot.Vector();
		}
	}

	OutTraceEnd = TraceStart + (AdjustedAimDir * MaxRange);
}

bool UAT_WaitForInteractableTarget::ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection,
	FVector AbilityCenter, float AbilityRange, FVector& ClippedPosition)
{
	FVector CameraToCenter = AbilityCenter - CameraLocation;
	float DotToCenter = FVector::DotProduct(CameraToCenter, CameraDirection);
	if (DotToCenter >= 0)		//If this fails, we're pointed away from the center, but we might be inside the sphere and able to find a good exit point.
	{
		float DistanceSquared = CameraToCenter.SizeSquared() - (DotToCenter * DotToCenter);
		float RadiusSquared = (AbilityRange * AbilityRange);
		if (DistanceSquared <= RadiusSquared)
		{
			float DistanceFromCamera = FMath::Sqrt(RadiusSquared - DistanceSquared);
			float DistanceAlongRay = DotToCenter + DistanceFromCamera;						//Subtracting instead of adding will get the other intersection point
			ClippedPosition = CameraLocation + (DistanceAlongRay * CameraDirection);		//Cam aim point clipped to range sphere
			return true;
		}
	}
	return false;
}

void UAT_WaitForInteractableTarget::LineTrace(FHitResult& HitResult, UWorld* World, const FVector& TraceStart,
	const FVector& TraceEnd, FName ProfileName, const FCollisionQueryParams& Params)
{
	check(World);

	HitResult = FHitResult();
	TArray<FHitResult> HitResults;
	World->LineTraceMultiByProfile(HitResults, TraceStart, TraceEnd, ProfileName, Params);

	HitResult.TraceStart = TraceStart;
	HitResult.TraceEnd = TraceEnd;

	if (HitResults.Num() > 0)
	{
		HitResult = HitResults[0];
	}
}

void UAT_WaitForInteractableTarget::UpdateInteractableOptions(const FInteractionQueryResult& InteractQuery,
	const TArray<TScriptInterface<IIInteractableTarget>>& InteractableTargets)
{
	TArray<FInteractionOption> NewOptions;

	for (const TScriptInterface<IIInteractableTarget>& InteractiveTarget : InteractableTargets)
	{
		TArray<FInteractionOption> TempOptions;
		FInteractionOptionBuilder InteractionBuilder(InteractiveTarget, TempOptions);
		InteractiveTarget->GatherInteractionOptions(InteractQuery, InteractionBuilder);

		for (FInteractionOption& Option : TempOptions)
		{
			FGameplayAbilitySpec* InteractionAbilitySpec = nullptr;

			// Method 2 - Target has its own ability system
			if (Option.TargetAbilitySystem && Option.TargetInteractionAbilityHandle.IsValid())
			{
				InteractionAbilitySpec = Option.TargetAbilitySystem->FindAbilitySpecFromHandle(Option.TargetInteractionAbilityHandle);
			}
			// Method 1 - We grant the ability to ourselves
			else if (Option.AbilityToGrant)
			{
				// Find the spec
				InteractionAbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(Option.AbilityToGrant);

				if (InteractionAbilitySpec)
				{
					// Stamp the resolved handle back onto the option
					Option.TargetAbilitySystem = AbilitySystemComponent.Get();
					Option.TargetInteractionAbilityHandle = InteractionAbilitySpec->Handle;
				}
			}

			if (InteractionAbilitySpec)
			{
				// Filter any options that we can't activate right now for whatever reason.
				if (InteractionAbilitySpec->Ability->CanActivateAbility(InteractionAbilitySpec->Handle, AbilitySystemComponent->AbilityActorInfo.Get()))
				{
					NewOptions.Add(Option);
				}
			}
		}
	}

	bool bOptionsChanged = false;
	if (NewOptions.Num() == CurrentOptions.Num())
	{
		NewOptions.Sort();

		for (int OptionIndex = 0; OptionIndex < NewOptions.Num(); OptionIndex++)
		{
			const FInteractionOption& NewOption = NewOptions[OptionIndex];
			const FInteractionOption& CurrentOption = CurrentOptions[OptionIndex];

			if (NewOption != CurrentOption)
			{
				bOptionsChanged = true;
				break;
			}
		}
	}
	else
	{
		bOptionsChanged = true;
	}

	if (bOptionsChanged)
	{
		CurrentOptions = NewOptions;
		InteractableObjectsChanged.Broadcast(CurrentOptions);
	}
}
