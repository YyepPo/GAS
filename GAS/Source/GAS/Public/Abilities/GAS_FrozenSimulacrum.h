#pragma once

#include "CoreMinimal.h"
#include "GAS_BaseAbility.h"
#include "GAS_FrozenSimulacrum.generated.h"

USTRUCT(BlueprintType)
struct FLeapMontages
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly) UAnimMontage* Fwd;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* FwdLeft;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* FwdRight;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* Bwd;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* BwdLeft;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* BwdRight;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* Left;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* Right;
};

UCLASS()
class GAS_API UGAS_FrozenSimulacrum : public UGAS_BaseAbility
{
	GENERATED_BODY()

	UGAS_FrozenSimulacrum();

public:

virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION(BlueprintCallable,BlueprintImplementableEvent)
	void ApplyForce(const FVector& Direction);
	
private:

	UAnimMontage* GetMontageForDirection(float ForwardDot,float RightDot);

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	FLeapMontages Montages;

	UFUNCTION()
	void OnMontageCompleted();

	UPROPERTY(EditDefaultsOnly, Category = "Leap")
	float LeapDistance = 1200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Leap")
	float LaunchZForce = 400.f;
};
