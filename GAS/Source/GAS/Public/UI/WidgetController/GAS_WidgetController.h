#pragma once

#include "CoreMinimal.h"
//#include "Data/GAS_AbilityInfoClass.h"
#include "Data/AbilityInfo.h"
#include "UObject/Object.h"
#include "GAS_WidgetController.generated.h"

class UGAS_AbilityInfo;
class UGAS_AbilitySystemComponent;
class UAbilitySystemComponent;
class UGAS_AttributeSetBase;
class UAttributeSet;
class AGASPlayerController;
class APlayerState;
class AGAS_PlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChangedSignature, int32, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityInfoSignature, const FGASAbilityInfo&, Info);

USTRUCT(BlueprintType)
struct FGAS_WidgetControllerParams
{
	GENERATED_BODY()

	FGAS_WidgetControllerParams() {}
	FGAS_WidgetControllerParams(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
	: PlayerController(PC), PlayerState(PS), AbilitySystemComponent(ASC), AttributeSet(AS) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerState> PlayerState = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAttributeSet> AttributeSet = nullptr;
};

UCLASS()
class GAS_API UGAS_WidgetController : public UObject
{
	GENERATED_BODY()

public:

	
	UGAS_WidgetController();

	UFUNCTION(BlueprintCallable)
	void SetWidgetControllerParams(const FGAS_WidgetControllerParams& WCParams);

	UFUNCTION(BlueprintCallable)
	virtual void BroadcastInitialValues();
	virtual void BindCallbacksToDependencies();

	UPROPERTY(BlueprintAssignable, Category="GAS|Messages")
	FAbilityInfoSignature AbilityInfoDelegate;

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Data")
	TObjectPtr<UAbilityInfo> AbilityInfo;
	
	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<APlayerState> PlayerState;

	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<AGASPlayerController> GASPlayerController;

	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<AGAS_PlayerState> GASPlayerState;

	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<UGAS_AbilitySystemComponent> GASAbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<UGAS_AttributeSetBase> GASAttributeSet;

	AGASPlayerController* GetGASPC();
	AGAS_PlayerState* GetGASPS();
	UGAS_AbilitySystemComponent* GetGASASC();
	UGAS_AttributeSetBase* GetGASAS();
};
