#include "Abilities/GAS_Hoarfrost.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

UGAS_Hoarfrost::UGAS_Hoarfrost()
{
	
}

void UGAS_Hoarfrost::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                     const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo,true,false);
		return;
	}
	
	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (Character)
	{
		Character->GetCharacterMovement()->DisableMovement();
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, HoarfrostMontage);

	MontageTask->OnCompleted.AddDynamic(this, &UGAS_Hoarfrost::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGAS_Hoarfrost::OnMontageCompleted);
	MontageTask->ReadyForActivation();

	if (HoarfrostMontage)
	{
		APlayerController* Controller =	Cast<APlayerController>(
		Cast<ACharacter>(GetAvatarActorFromActorInfo())->GetController());
		if (Controller)
		{
			Controller->ClientStartCameraShake(HoarfrostCameraShake);
		}	
	}


	if (WindParticle)
	{
		FVector SpawnLocation = GetAvatarActorFromActorInfo()->GetActorLocation();
		SpawnLocation.Z -= 25.f;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),WindParticle,SpawnLocation,FRotator(0,0,0));
	}
}

void UGAS_Hoarfrost::OnMontageCompleted()
{
	// Restore movement
	const ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character)
	{
		Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}

	EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,false);
}
