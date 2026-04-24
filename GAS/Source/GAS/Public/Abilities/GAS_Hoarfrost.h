#pragma once

#include "CoreMinimal.h"
#include "GAS_BaseAbility.h"
#include "GAS_Hoarfrost.generated.h"

UCLASS()
class GAS_API UGAS_Hoarfrost : public UGAS_BaseAbility
{
	GENERATED_BODY()

	UGAS_Hoarfrost();
	
protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:

	UPROPERTY(EditAnywhere)
	UAnimMontage* HoarfrostMontage;

	UPROPERTY(EditAnywhere)
	UParticleSystem* WindParticle;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UCameraShakeBase> HoarfrostCameraShake;
	
	UFUNCTION()
	void OnMontageCompleted();
};
