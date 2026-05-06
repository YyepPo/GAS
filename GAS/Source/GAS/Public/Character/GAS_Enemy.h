// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS_BaseCharacter.h"
#include "UI/WidgetController/GAS_OverlayWidgetController.h"
#include "GameplayEffect.h"
#include "GAS_Enemy.generated.h"

struct FActiveGameplayEffectHandle;
struct FGameplayEffectSpec;
class FOnAttributeChangedSignature;
class UWidgetComponent;
class UOverlayWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStackCountChanged,int32,NewCount);

UCLASS()
class GAS_API AGAS_Enemy : public AGAS_BaseCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGAS_Enemy();

	virtual int32 GetPlayerLevel_Implementation() override;

	virtual void Die(const FVector& DeathImpulse) override;

	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnMaxHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FOnStackCountChanged OnStackCountChanged;
	
protected:

	virtual void BeginPlay() override;

	virtual void InitAbilityInfo() override;
	virtual void ApplyDefaultAttributes() const override;
	
private:

	int32 Level = 1;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> HealthWidgetComponent;
	
	void StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	void ListenForStackCountChange();
	
	// Mirrors what the async task does internally
	void OnActiveGameplayEffectAdded(UAbilitySystemComponent* ASC,
		const FGameplayEffectSpec& Spec,
		FActiveGameplayEffectHandle Handle);
	
	void OnAnyGameplayEffectRemovedEvent(const FActiveGameplayEffect& Effect);

	void OnHitStackChanged(FActiveGameplayEffectHandle Handle,
		int32 NewCount, int32 OldCount);

	FGameplayTag WatchedStackTag;

	
};
