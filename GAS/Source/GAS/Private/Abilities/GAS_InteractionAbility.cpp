// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/GAS_InteractionAbility.h"

#include "AbilitySystemComponent.h"
#include "Interaction/AT_GrantNearbyInteraction.h"
#include "Interaction/IInteractableTarget.h"
#include "Interaction/InteractionOption.h"

UGAS_InteractionAbility::UGAS_InteractionAbility(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGAS_InteractionAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                              const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                              const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Only run the sphere detection on the server
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (AbilitySystemComponent && AbilitySystemComponent->GetOwnerRole() == ROLE_Authority)
	{
		UAT_GrantNearbyInteraction* GrantTask =	UAT_GrantNearbyInteraction::CreateGrantNearbyInteraction(this,ScanRadius,ScanRate);
		GrantTask->ReadyForActivation();	
	}
}

void UGAS_InteractionAbility::UpdateInteraction(const TArray<FInteractionOption>& InInteractionOptions)
{
	CurrentOptions = InInteractionOptions;
}

bool UGAS_InteractionAbility::TriggerInteraction()
{
	if (CurrentOptions.Num() == 0)
	{
		return false;
	}

	UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentFromActorInfo();
	if (AbilitySystem)
	{
		const FInteractionOption& InteractionOption = CurrentOptions[0];

		AActor* Instigator = GetAvatarActorFromActorInfo();
		AActor* InteractableTargetActor = GetActorFromInteractableTarget(InteractionOption.InteractableTarget);

		// Allow the target to customize the event data we're about to pass in, in case the ability needs custom data
		// that only the actor knows.
		FGameplayEventData Payload;
		Payload.EventTag = FGameplayTag::RequestGameplayTag("Ability.Interaction.Activate");
		Payload.Instigator = Instigator;
		Payload.Target = InteractableTargetActor;

		// If needed we allow the interactable target to manipulate the event data so that for example, a button on the wall
		// may want to specify a door actor to execute the ability on, so it might choose to override Target to be the
		// door actor.
		InteractionOption.InteractableTarget->CustomizeInteractionEventData(FGameplayTag::RequestGameplayTag("Ability.Interaction.Activate"), Payload);

		// Grab the target actor off the payload we're going to use it as the 'avatar' for the interaction, and the
		// source InteractableTarget actor as the owner actor.
		AActor* TargetActor = const_cast<AActor*>(ToRawPtr(Payload.Target));

		// The actor info needed for the interaction.
		FGameplayAbilityActorInfo ActorInfo;
		ActorInfo.InitFromActor(InteractableTargetActor, TargetActor, InteractionOption.TargetAbilitySystem);

		// Trigger the ability using event tag.
		return InteractionOption.TargetAbilitySystem->TriggerAbilityFromGameplayEvent(
			InteractionOption.TargetInteractionAbilityHandle,
			&ActorInfo,
			FGameplayTag::RequestGameplayTag("Ability.Interaction.Activate"),
			&Payload,
			*InteractionOption.TargetAbilitySystem
		);
	}
	return false;
}

AActor* UGAS_InteractionAbility::GetActorFromInteractableTarget(
	TScriptInterface<IIInteractableTarget> InteractableTarget)
{
	if (UObject* Object = InteractableTarget.GetObject())
	{
		if (AActor* Actor = Cast<AActor>(Object))
		{
			return Actor;
		}
		else if (UActorComponent* ActorComponent = Cast<UActorComponent>(Object))
		{
			return ActorComponent->GetOwner();
		}
		else
		{
			unimplemented();
		}
	}

	return nullptr;
}
