#include "Abilities/GAS_GlacialCharge.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Actors/GAS_IcyTrail.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"


UGAS_GlacialCharge::UGAS_GlacialCharge()
{
}



void UGAS_GlacialCharge::OnMontageCompleted()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());

	// Restore normal movement
	Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	// Stop timers
	GetWorld()->GetTimerManager().ClearTimer(TrailSpawnTimer);
	GetWorld()->GetTimerManager().ClearTimer(DamageTimer);

	bIsCharging = false;

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGAS_GlacialCharge::PerformChargeSweep()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    
    TArray<FHitResult> HitResults;
    FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(80.f, 90.f);

    GetWorld()->SweepMultiByChannel(
        HitResults,
        Character->GetActorLocation(),
        Character->GetActorLocation() + Character->GetActorForwardVector() * 50.f,
        FQuat::Identity,
        ECC_Pawn,
        CollisionShape
    );

    TArray<AActor*> AlreadyHit;

    for (FHitResult& Hit : HitResults)
    {
        AActor* HitActor = Hit.GetActor();
        if (!HitActor || HitActor == Character || AlreadyHit.Contains(HitActor)) continue;

        AlreadyHit.Add(HitActor);

        UAbilitySystemComponent* TargetASC =
            UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);

        if (TargetASC)
        {
            FGameplayEffectContextHandle Context =
                GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
            Context.AddSourceObject(Character);

            // Apply damage
            FGameplayEffectSpecHandle DamageSpec =
                GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(
                    DamageEffectClass, GetAbilityLevel(), Context);
            GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(
                *DamageSpec.Data.Get(), TargetASC);

            // Push enemy aside
            if (ACharacter* EnemyCharacter = Cast<ACharacter>(HitActor))
            {
                const FVector PushDirection = FVector::CrossProduct(
                    Character->GetActorForwardVector(), FVector::UpVector).GetSafeNormal();
                
                // Alternate push left/right based on which side enemy is on
                const FVector ToEnemy = (HitActor->GetActorLocation() - Character->GetActorLocation()).GetSafeNormal();
                const float Side = FVector::DotProduct(ToEnemy, Character->GetActorRightVector());
                const FVector FinalPush = (Side > 0.f ? PushDirection : -PushDirection) * PushForce
                    + FVector(0.f, 0.f, PushZForce);

                EnemyCharacter->LaunchCharacter(FinalPush, true, true);

                // Fire hit gameplay cue
                FGameplayCueParameters CueParams;
                CueParams.Location = Hit.ImpactPoint;
                CueParams.Normal = Hit.ImpactNormal;
                GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(
                    FGameplayTag::RequestGameplayTag("GameplayCue.GlacialCharge.Impact"),
                    CueParams);
            }
        }
    }
	
}

void UGAS_GlacialCharge::SpawnTrailSegment()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());

	AGAS_IcyTrail* Trail = GetWorld()->SpawnActor<AGAS_IcyTrail>(
		IcyTrailClass,
		Character->GetActorLocation(),
		Character->GetActorRotation()
	);

	if (Trail)
	{
		SpawnedTrailSegments.Add(Trail);
	}
}
