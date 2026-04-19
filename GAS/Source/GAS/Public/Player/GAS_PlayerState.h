#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "GAS_PlayerState.generated.h"

class UGAS_AttributeSetBase;
class UAbilitySystemComponent;

UCLASS()
class GAS_API AGAS_PlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

	AGAS_PlayerState();

protected:
	
	virtual void BeginPlay() override;	
	
public:

	/** IAbilitySystemInterface function */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	/** IAbilitySystemInterface END */
	
	UAttributeSet* GetAttributeSet() const {return AttributeBase;} 
	
protected:
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeBase;
	
	///
	/// Callbacks for attribute changes
	//
	
	virtual void OnHealthChanged(const FOnAttributeChangeData& Data);
	virtual void OnStaminaChanged(const FOnAttributeChangeData& Data);
	virtual void OnManaChanged(const FOnAttributeChangeData& Data);
	
	///
	/// Delegates for attribute changes
	//
	
	FDelegateHandle HealthChangeDelegateHandle;
	FDelegateHandle StaminaChangedDelegateHandle;
	FDelegateHandle ManaChangedDelegateHandle;
	
	
};


