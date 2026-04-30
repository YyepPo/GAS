#include "UI/WidgetController/GAS_AttributeWidgetController.h"
#include "AbilityComponent/GAS_AbilitySystemComponent.h"
#include "Attributes/GAS_AttributeSetBase.h"
#include "Player/GAS_PlayerState.h"


void UGAS_AttributeWidgetController::BindCallbacksToDependencies()
{
	for (const auto Pair : GetGASAS()->TagsToAttributes)
	{
		GetGASASC()->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda([this,Pair](const FOnAttributeChangeData& Data)
		{
			BroadcastAttributeChange(Pair.Key,Pair.Value());			
		});
	}

	GetGASPS()->OnAttributePointsChangedDelegate.AddLambda([this](int32 Point)
	{
		AttributePointsChanged.Broadcast(Point);
	});
}

void UGAS_AttributeWidgetController::BroadcastInitialValues()
{
	UGAS_AttributeSetBase* GAS_AttributeSet = CastChecked<UGAS_AttributeSetBase>(AttributeSet);
	
	for (const auto Pair : GAS_AttributeSet->TagsToAttributes)
	{
		BroadcastAttributeChange(Pair.Key,Pair.Value());
	}
	
	AttributePointsChanged.Broadcast(GetGASPS()->GetAttributePoints());
}

void UGAS_AttributeWidgetController::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	UGAS_AbilitySystemComponent* GASAC = CastChecked<UGAS_AbilitySystemComponent>(AbilitySystemComponent);
	GASAC->SpendAttributePoint(AttributeTag);
}

void UGAS_AttributeWidgetController::BroadcastAttributeChange(const FGameplayTag& AttributeTag,
                                                              const FGameplayAttribute& Attribute)
{
check(AttributeInfo);

	FAuraAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag);
	Info.Value = Attribute.GetNumericValue(GetGASAS());
	OnAttributeChange.Broadcast(Info);
}

