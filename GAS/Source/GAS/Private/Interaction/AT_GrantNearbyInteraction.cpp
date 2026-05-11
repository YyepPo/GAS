// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/AT_GrantNearbyInteraction.h"

#include "AbilitySystemComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Character.h"
#include "Interaction/IInteractableTarget.h"
#include "Interaction/InteractionQuery.h"
#include "Kismet/GameplayStatics.h"

UAT_GrantNearbyInteraction* UAT_GrantNearbyInteraction::CreateGrantNearbyInteraction(UGameplayAbility* OwningAbility,
                                                                                     float ScanRadius, float ScanRate)
{
	UAT_GrantNearbyInteraction* Object = NewAbilityTask<UAT_GrantNearbyInteraction>(OwningAbility);
	
		Object->ScanRadius = ScanRadius;
		Object->ScanRate = ScanRate;
		return Object;
}

void UAT_GrantNearbyInteraction::Activate()
{
	SetWaitingOnAvatar();

	GetWorld()->GetTimerManager().SetTimer(ScanTimerHandle,this,&UAT_GrantNearbyInteraction::OnScanTimerHandleFinished,ScanRate,true);
	ReadyForActivation();
}

void UAT_GrantNearbyInteraction::OnDestroy(bool bInOwnerFinished)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ScanTimerHandle);
	}
	
	Super::OnDestroy(bInOwnerFinished);
}

void UAT_GrantNearbyInteraction::OnScanTimerHandleFinished()
{
	UWorld* World = GetWorld();
	AActor* OwningActor =	GetAvatarActor();
	if (World == nullptr || OwningActor == nullptr)
	{
		return;
	}

#if ENABLE_DRAW_DEBUG

	DrawDebugSphere(World,OwningActor->GetActorLocation(),ScanRadius,20,FColor::Green,false,ScanRate);
	
#endif

	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByChannel(Overlaps,OwningActor->GetActorLocation(),FQuat::Identity,ECollisionChannel::ECC_Visibility,FCollisionShape::MakeSphere(ScanRadius));

	if (Overlaps.Num() > 0)
	{
		FInteractionQueryResult Result;
		Result.InteractingActor = OwningActor;
		Result.InteractingController = Cast<ACharacter>(OwningActor)->GetController();

		// Some actors might not use this interface, so get all the overlapped actors and seperate those that have IIInteractableTargets
		TArray<TScriptInterface<IIInteractableTarget>> InteractableTargets;
		for (FOverlapResult& Overlap : Overlaps)
		{
			TScriptInterface<IIInteractableTarget> InteractableTarget(Overlap.GetActor());
			if (InteractableTarget)
			{
				InteractableTargets.Add(InteractableTarget);
			}

			TScriptInterface<IIInteractableTarget> InteractableTargett(Overlap.GetComponent());
			if (InteractableTargett)
			{
				InteractableTargets.Add(InteractableTargett);
			}
		}

		// This allows that an item can grant multiple abilities/items to the interacting player
		TArray<FInteractionOption> InteractionOptions;
		for (TScriptInterface<IIInteractableTarget>& InteractableTarget : InteractableTargets)
		{
			FInteractionOptionBuilder InteractionOptionBuilder = FInteractionOptionBuilder(InteractableTarget,InteractionOptions);
			InteractableTarget->GatherInteractionOptions(Result,InteractionOptionBuilder);
		}

		for (FInteractionOption& Option : InteractionOptions)
		{
			if (Option.AbilityToGrant == nullptr)
			{
				continue;
			}

			FObjectKey Key = Option.AbilityToGrant;
			if (!InteractionAbilityCache.Find(Key))
			{
				FGameplayAbilitySpec Spec(Option.AbilityToGrant, 1, INDEX_NONE, this);
				FGameplayAbilitySpecHandle Handle = AbilitySystemComponent->GiveAbility(Spec);
				InteractionAbilityCache.Add(Key, Handle);
			}
		}
	}
}
