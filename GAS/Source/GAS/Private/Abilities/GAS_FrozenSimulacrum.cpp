// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/GAS_FrozenSimulacrum.h"

#include "AbilitySystemComponent.h"
#include "GAS_GameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionJumpForce.h"
#include "GameFramework/Character.h"

UGAS_FrozenSimulacrum::UGAS_FrozenSimulacrum()
{
}

void UGAS_FrozenSimulacrum::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                            const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                            const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	// Get movement input direction relative to character
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	const FVector InputVector = Character->GetLastMovementInputVector();
    
	// Get direction relative to camera/character forward
	const FVector ForwardVector = Character->GetActorForwardVector();
	const FVector RightVector = Character->GetActorRightVector();

	const float ForwardDot = FVector::DotProduct(InputVector.GetSafeNormal(), ForwardVector);
	const float RightDot = FVector::DotProduct(InputVector.GetSafeNormal(), RightVector);

	UAnimMontage* LeapMontage = GetMontageForDirection(ForwardDot, RightDot);
	FVector LeapDirection = InputVector.IsNearlyZero() ? ForwardVector : InputVector.GetSafeNormal();
		
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, LeapMontage);
	MontageTask->OnCompleted.AddDynamic(this, &UGAS_FrozenSimulacrum::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGAS_FrozenSimulacrum::OnMontageCompleted);
	MontageTask->ReadyForActivation();
	
	const FVector LaunchVelocity = LeapDirection * LeapDistance + FVector(0.f, 0.f, LaunchZForce);
	//Character->LaunchCharacter(LaunchVelocity, true, true);

	ApplyForce(LeapDirection);
	
	FGAS_GameplayTags::Get();
	const FGAS_GameplayTags& GameplayTags = FGAS_GameplayTags::Get();
	FGameplayTagContainer BlockedTags;
	BlockedTags.AddTag(GameplayTags.Player_Block_CursorTrace);
	BlockedTags.AddTag(GameplayTags.Player_Block_InputHeld);
	BlockedTags.AddTag(GameplayTags.Player_Block_InputPressed);
	BlockedTags.AddTag(GameplayTags.Player_Block_InputReleased);

	UAbilitySystemComponent* AbilityComp  = GetAbilitySystemComponentFromActorInfo();
	if (AbilityComp)
	{
		AbilityComp->AddLooseGameplayTags(BlockedTags);
	}
}

UAnimMontage* UGAS_FrozenSimulacrum::GetMontageForDirection(float ForwardDot, float RightDot)
{
	const float DiagonalThreshold = 0.5f;

	if (ForwardDot > DiagonalThreshold)
	{
		if (RightDot > DiagonalThreshold)       return Montages.FwdRight;
		else if (RightDot < -DiagonalThreshold) return Montages.FwdLeft;
		else                                     return Montages.Fwd;
	}
	else if (ForwardDot < -DiagonalThreshold)
	{
		if (RightDot > DiagonalThreshold)       return Montages.BwdRight;
		else if (RightDot < -DiagonalThreshold) return Montages.BwdLeft;
		else                                     return Montages.Bwd;
	}
	else
	{
		if (RightDot > 0.f) return Montages.Right;
		else                return Montages.Left;
	}	
}

void UGAS_FrozenSimulacrum::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo, true, false);
}
