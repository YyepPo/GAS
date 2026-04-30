#pragma once

#include "CoreMinimal.h"
#include "GAS_WidgetController.h"
#include "Data/AttributeInformation.h"
#include "GAS_AttributeWidgetController.generated.h"

struct FGameplayAttribute;
struct FAuraAttributeInfo;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChanged, const FAuraAttributeInfo&, AttributeInfo);

UCLASS(BlueprintType, Blueprintable)
class GAS_API UGAS_AttributeWidgetController : public UGAS_WidgetController
{
	GENERATED_BODY()

public:

	virtual void BindCallbacksToDependencies() override;
	virtual void BroadcastInitialValues() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAttributeInformation> AttributeInfo;

	UPROPERTY(Blueprintassignable)
	FOnAttributeChanged OnAttributeChange;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerStatChangedSignature AttributePointsChanged;

	UFUNCTION(BlueprintCallable)
	void UpgradeAttribute(const FGameplayTag& AttributeTag);
	
private:

	void BroadcastAttributeChange(const FGameplayTag& AttributeTag,const FGameplayAttribute& Attribute);

};
