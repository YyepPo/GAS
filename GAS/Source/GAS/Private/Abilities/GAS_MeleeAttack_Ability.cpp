// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/GAS_MeleeAttack_Ability.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Task/GAS_AbilityTask_MeleeTrace.h"
#include "AbilityComponent/GAS_FunctionLibrary.h"
#include "Data/GAS_AbilityTypes.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Interface/CombatInterface.h"


void UGAS_MeleeAttack_Ability::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                               const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                               const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    
    CommitAbility(Handle, ActorInfo, ActivationInfo);

    // Play 
    UAbilityTask_PlayMontageAndWait* MontageTask =
       UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
           this,
           NAME_None,
           AttackMontage
       );

    MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
    MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCompleted);
    MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCompleted);
    MontageTask->Activate();

    // Wait for anim notify to open the window
    UAbilityTask_WaitGameplayEvent* WaitOpen =
        UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
            this,
            FGameplayTag::RequestGameplayTag("Event.HitWindow.Open"),
            nullptr, true);
    WaitOpen->EventReceived.AddDynamic(this, &ThisClass::OnHitWindowOpen);
    WaitOpen->Activate();
}

void UGAS_MeleeAttack_Ability::EndAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled)
{
    if (MeleeTraceTask)
    {
        MeleeTraceTask->EndTask();
        MeleeTraceTask = nullptr;
    }
    
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGAS_MeleeAttack_Ability::OnHitWindowOpen(FGameplayEventData Payload)
{
    // Get the weapon mesh from the avatar (cast to your combat interface / character)
    AActor* Avatar = GetAvatarActorFromActorInfo();
    USkeletalMeshComponent* OwnerMesh =
         Avatar->FindComponentByClass<USkeletalMeshComponent>();

    if (!OwnerMesh) { EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false); return; }

    MeleeTraceTask = UGAS_AbilityTask_MeleeTrace::CreateMeleeTrace(
        this,
        OwnerMesh,       // pass the owner's mesh
        TipSocket,       // e.g. "WeaponTip"   — socket on the sword bone
        RootSocket,      // e.g. "WeaponRoot"  — socket on the sword bone
        SweepRadius
    );
    MeleeTraceTask->OnHit.AddDynamic(this, &ThisClass::OnTraceHit);
    MeleeTraceTask->Activate();
}

void UGAS_MeleeAttack_Ability::OnHitWindowClose(FGameplayEventData Payload)
{
    if (MeleeTraceTask)
    {
        MeleeTraceTask->EndTask();
        MeleeTraceTask = nullptr;
    }
}

void UGAS_MeleeAttack_Ability::OnMontageCompleted()
{
    // Safety net — close window if anim ends before notify fires
    OnHitWindowClose(FGameplayEventData{});
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGAS_MeleeAttack_Ability::OnTraceHit(const FHitResult& HitResult)
{
    AActor* TargetActor = HitResult.GetActor();
    if (!TargetActor) return;

    UAbilitySystemComponent* TargetASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    if (!TargetASC) return;

    AActor* SourceActor = GetAvatarActorFromActorInfo();

    // Direction from attacker to victim for impulse/knockback
    const FVector ImpulseDirection =
        (TargetActor->GetActorLocation() - SourceActor->GetActorLocation()).GetSafeNormal();

    // Build params — matches your exact FDamageEffectParams fields
    FDamageEffectParams Params;
    Params.WorldContextObject          = SourceActor;
    Params.DamageGameplayEffectClass   = DamageEffectClass;
    Params.SourceAbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
    Params.TargetAbilitySystemComponent = TargetASC;
    Params.BaseDamage                  = BaseDamage;
    Params.AbilityLevel                = GetAbilityLevel();
    Params.DamageType                  = DamageType;
    Params.DeathImpulseMagnitude       = DeathImpulseMagnitude;
    Params.DeathImpulse                = ImpulseDirection * DeathImpulseMagnitude;
    Params.KnockbackForceMagnitude     = KnockbackForceMagnitude;
    Params.KnockbackChance             = KnockbackChance;
    Params.KnockbackForce              = ImpulseDirection * KnockbackForceMagnitude;

    UGAS_FunctionLibrary::ApplyDamageEffect(Params);
}
