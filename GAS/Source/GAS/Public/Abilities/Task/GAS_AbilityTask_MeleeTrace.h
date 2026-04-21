#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "GAS_AbilityTask_MeleeTrace.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMeleeHit, const FHitResult&, HitResult);

UCLASS()
class GAS_API UGAS_AbilityTask_MeleeTrace : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnMeleeHit OnHit;

	UFUNCTION()
	static UGAS_AbilityTask_MeleeTrace* CreateMeleeTrace(
		UGameplayAbility* OwningAbility,
		USkeletalMeshComponent* InWeaponMesh,
		FName InTipSocket,
		FName InRootSocket,
		float InRadius);

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

private:
	
	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	FName TipSocket;
	FName RootSocket;
	float Radius;

	FVector PreviousTipLocation;
	FVector PreviousRootLocation;

	// One hit per actor per swing
	TSet<TWeakObjectPtr<AActor>> HitActorsThisSwing;
};
