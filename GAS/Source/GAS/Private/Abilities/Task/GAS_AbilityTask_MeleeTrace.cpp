// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/Task/GAS_AbilityTask_MeleeTrace.h"

UGAS_AbilityTask_MeleeTrace* UGAS_AbilityTask_MeleeTrace::CreateMeleeTrace(
    UGameplayAbility* OwningAbility,
    USkeletalMeshComponent* InWeaponMesh,
    FName InTipSocket,
    FName InRootSocket,
    float InRadius)
{
    UGAS_AbilityTask_MeleeTrace* Task =
        NewAbilityTask<UGAS_AbilityTask_MeleeTrace>(OwningAbility);
    Task->WeaponMesh  = InWeaponMesh;
    Task->TipSocket   = InTipSocket;
    Task->RootSocket  = InRootSocket;
    Task->Radius      = InRadius;
    return Task;
}

void UGAS_AbilityTask_MeleeTrace::Activate()
{
    // Snapshot positions so first tick has a valid "previous"
    PreviousTipLocation  = WeaponMesh->GetSocketLocation(TipSocket);
    PreviousRootLocation = WeaponMesh->GetSocketLocation(RootSocket);

    bTickingTask = true; // AbilityTask system calls TickTask every frame
}

void UGAS_AbilityTask_MeleeTrace::TickTask(float DeltaTime)
{
    if (!WeaponMesh) { EndTask(); return; }

    const FVector CurrentTip  = WeaponMesh->GetSocketLocation(TipSocket);
    const FVector CurrentRoot = WeaponMesh->GetSocketLocation(RootSocket);

    // Half-length for the capsule = half the distance between sockets
    const float HalfHeight =
        FMath::Max(FVector::Dist(CurrentRoot, CurrentTip) * 0.5f, Radius);

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetAvatarActor());

    TArray<FHitResult> Hits;
    GetWorld()->SweepMultiByChannel(
        Hits,
        PreviousRootLocation,   // sweep from last frame root
        CurrentRoot,            // to this frame root
        FQuat::FindBetweenNormals(FVector::UpVector,
            (CurrentTip - CurrentRoot).GetSafeNormal()), // orient capsule along blade
        ECC_Pawn,
        FCollisionShape::MakeCapsule(Radius, HalfHeight),
        Params
    );

    for (const FHitResult& Hit : Hits)
    {
        AActor* HitActor = Hit.GetActor();
        if (!HitActor) continue;

        // Skip if already hit this actor this swing
        if (HitActorsThisSwing.Contains(HitActor)) continue;
        HitActorsThisSwing.Add(HitActor);

        if (ShouldBroadcastAbilityTaskDelegates())
        {
            OnHit.Broadcast(Hit);
        }
    }

    PreviousTipLocation  = CurrentTip;
    PreviousRootLocation = CurrentRoot;
}

void UGAS_AbilityTask_MeleeTrace::OnDestroy(bool bInOwnerFinished)
{
    HitActorsThisSwing.Empty();
    Super::OnDestroy(bInOwnerFinished);
}