#include "Abilities/GAS_IceRain.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "Abilities/Targeting/GAS_GroundTargetActor.h"
#include "Actors/GAS_IceRainSpike.h"
#include "GAS_GameplayTags.h"
#include "GameFramework/Pawn.h"
#include "GameplayEffect.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

UGAS_IceRain::UGAS_IceRain()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	TargetActorClass = AGAS_GroundTargetActor::StaticClass();
	IceRainSpikeClass = AGAS_IceRainSpike::StaticClass();

	static ConstructorHelpers::FClassFinder<UGameplayEffect> DamageEffectFinder(
		TEXT("/Game/GAS/Blueprints/GameplayEffect/GE_Damage"));
	if (DamageEffectFinder.Succeeded())
	{
		DamageEffectClass = DamageEffectFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UGameplayEffect> StunEffectFinder(
		TEXT("/Game/GAS/Blueprints/GameplayEffect/GE_Stun"));
	if (StunEffectFinder.Succeeded())
	{
		StunEffectClass = StunEffectFinder.Class;
	}
}

void UGAS_IceRain::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (ActorInfo == nullptr || ActorInfo->AvatarActor.IsValid() == false || TargetActorClass == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	BeginTargetingPhase();
}

void UGAS_IceRain::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpawnTimerHandle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGAS_IceRain::BeginTargetingPhase()
{
	TargetDataTask = UAbilityTask_WaitTargetData::WaitTargetData(
		this,
		NAME_None,
		EGameplayTargetingConfirmation::UserConfirmed,
		TargetActorClass);

	if (TargetDataTask == nullptr)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	TargetDataTask->ValidData.AddDynamic(this, &ThisClass::OnTargetDataReady);
	TargetDataTask->Cancelled.AddDynamic(this, &ThisClass::OnTargetDataCancelled);

	AGameplayAbilityTargetActor* SpawnedTargetActor = nullptr;
	if (TargetDataTask->BeginSpawningActor(this, TargetActorClass, SpawnedTargetActor))
	{
		if (AGAS_GroundTargetActor* GroundTargetActor = Cast<AGAS_GroundTargetActor>(SpawnedTargetActor))
		{
			GroundTargetActor->TargetRadius = TargetRadius;
		}

		TargetDataTask->FinishSpawningActor(this, SpawnedTargetActor);
	}

	TargetDataTask->ReadyForActivation();
}

void UGAS_IceRain::OnTargetDataReady(const FGameplayAbilityTargetDataHandle& TargetData)
{
	if (TargetData.Num() <= 0)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	const FGameplayAbilityTargetData* Data = TargetData.Get(0);
	if (Data == nullptr || Data->HasHitResult() == false)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	const FHitResult* HitResult = Data->GetHitResult();
	if (HitResult == nullptr)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	LockedTargetLocation = HitResult->ImpactPoint;
	BeginIceRain(LockedTargetLocation);
}

void UGAS_IceRain::OnTargetDataCancelled(const FGameplayAbilityTargetDataHandle& TargetData)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGAS_IceRain::BeginIceRain(const FVector& TargetLocation)
{
	LockedTargetLocation = TargetLocation;

	if (AActor* AvatarActor = GetAvatarActorFromActorInfo())
	{
		if (AvatarActor->HasAuthority())
		{
			SpawnIceSpike();

			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().SetTimer(
					SpawnTimerHandle,
					this,
					&ThisClass::SpawnIceSpike,
					SpawnInterval,
					true,
					SpawnInterval);
			}
		}
	}

	RainDurationTask = UAbilityTask_WaitDelay::WaitDelay(this, RainDuration);
	if (RainDurationTask)
	{
		RainDurationTask->OnFinish.AddDynamic(this, &ThisClass::OnRainDurationFinished);
		RainDurationTask->ReadyForActivation();
	}
}

void UGAS_IceRain::SpawnIceSpike()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UWorld* World = GetWorld();
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (AvatarActor == nullptr || AvatarActor->HasAuthority() == false || World == nullptr || IceRainSpikeClass == nullptr || SourceASC == nullptr)
	{
		return;
	}

	const FVector SpawnLocation = GetRandomSpikeSpawnLocation();
	const FTransform SpawnTransform(FRotator(0.f, 90.f, 0.f), SpawnLocation);

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = AvatarActor;
	SpawnParameters.Instigator = Cast<APawn>(AvatarActor);
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AGAS_IceRainSpike* Spike = World->SpawnActorDeferred<AGAS_IceRainSpike>(
		IceRainSpikeClass,
		SpawnTransform,
		AvatarActor,
		SpawnParameters.Instigator,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (Spike == nullptr)
	{
		return;
	}

	const FGameplayTag EffectiveDamageType = DamageType.IsValid()
		? DamageType
		: FGAS_GameplayTags::Get().Damage_Physical;

	Spike->InitializeSpike(
		SourceASC,
		DamageEffectClass,
		StunEffectClass,
		GetAbilityLevel(),
		BaseDamage,
		EffectiveDamageType,
		FVector::DownVector * SpikeFallSpeed);

	Spike->FinishSpawning(SpawnTransform);
}

FVector UGAS_IceRain::GetRandomSpikeSpawnLocation() const
{
	const float Angle = FMath::FRandRange(0.f, 2.f * UE_PI);
	const float Distance = TargetRadius * FMath::Sqrt(FMath::FRand());
	const FVector Offset(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, SpawnHeight);
	return LockedTargetLocation + Offset;
}

void UGAS_IceRain::OnRainDurationFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
