// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Actor.h"
#include "Interface/CharacterInterface.h"
#include "Interface/CombatInterface.h"
#include "UI/WidgetController/GAS_OverlayWidgetController.h"
#include "GAS_IcyCone.generated.h"

class UGameplayEffect;
class UWidgetComponent;
class UAttributeSet;



UCLASS()
class GAS_API AGAS_IcyCone : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	
	AGAS_IcyCone();

	
public:

	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnHealthChanged;
	
protected:
	
	virtual void BeginPlay() override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
private:

	UPROPERTY(EditAnywhere,BlueprintReadWrite,meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> HealthWidgetComponent;

	UPROPERTY(BlueprintReadWrite,meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> VitalAttribute;
};
