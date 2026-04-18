
#pragma once

#include "CoreMinimal.h"
#include "GAS_BaseAbility.h"
#include "GAS_PassiveAbility.generated.h"

UCLASS()
class GAS_API UGAS_PassiveAbility : public UGAS_BaseAbility
{
	GENERATED_BODY()
	
public:
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	void ReceiveDeactivate(const FGameplayTag& AbilityTag);

};
