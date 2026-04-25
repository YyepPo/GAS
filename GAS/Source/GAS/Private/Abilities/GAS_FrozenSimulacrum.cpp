#include "Abilities/GAS_FrozenSimulacrum.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilityComponent/GAS_FunctionLibrary.h"
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

	SpawnClone(Character->GetActorLocation(),Character->GetActorRotation());
	
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, LeapMontage);
	MontageTask->OnCompleted.AddDynamic(this, &UGAS_FrozenSimulacrum::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGAS_FrozenSimulacrum::OnMontageCompleted);
	MontageTask->ReadyForActivation();
	
	ApplyForce(LeapDirection);

	UGAS_FunctionLibrary::ApplyBlockMovementTag(GetAbilitySystemComponentFromActorInfo());
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

void UGAS_FrozenSimulacrum::SpawnClone(const FVector& Location,const FRotator& Rotation)
{
	if (!CloneClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGAS_FrozenSimulacrum: CloneClass is not valid."));
		return;
	}

	FHitResult Hit;

	const FVector Start = Location + FVector(0.f, 0.f, 100.f);
	const FVector End = Location - FVector(0.f, 0.f, 1000.f);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetAvatarActorFromActorInfo());

	FVector SpawnLocation = Location;

	if (GetWorld()->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		ECC_Visibility, 
		QueryParams))
	{
		SpawnLocation = Hit.ImpactPoint;
		SpawnLocation.Z += 2.f;
	}

	FActorSpawnParameters Params;
	Params.Owner = GetAvatarActorFromActorInfo();

	GetWorld()->SpawnActor<AActor>(
		CloneClass,
		SpawnLocation,
		Rotation,
		Params
	);

#if WITH_EDITOR
	DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, 3.f);
	DrawDebugSphere(GetWorld(), SpawnLocation, 20.f, 12, FColor::Green, false, 3.f);
#endif
}

void UGAS_FrozenSimulacrum::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo, true, false);
}
