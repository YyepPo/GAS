#include "UI/GAS_HUD.h"
#include "UI/Widget/OverlayWidget.h"
#include "UI/WidgetController/GAS_AttributeWidgetController.h"
#include "UI/WidgetController/GAS_OverlayWidgetController.h"

void AGAS_HUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(OverlayWidgetClass, TEXT("Overlay Widget Class uninitialized, please fill out BP_AuraHUD"));
	checkf(OverlayWidgetControllerClass, TEXT("Overlay Widget Controller Class uninitialized, please fill out BP_AuraHUD"));
	
	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
	OverlayWidget = Cast<UOverlayWidget>(Widget);
	
	const FGAS_WidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
	UGAS_OverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);

	OverlayWidget->SetWidgetController(WidgetController);
	WidgetController->BroadcastInitialValues();
	Widget->AddToViewport();
}

UGAS_OverlayWidgetController* AGAS_HUD::GetOverlayWidgetController(const FGAS_WidgetControllerParams& WCParams)
{
	if (OverlayWidgetController == nullptr)
	{
		OverlayWidgetController = NewObject<UGAS_OverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(WCParams);
		OverlayWidgetController->BindCallbacksToDependencies();
	}
	return OverlayWidgetController;
}

UGAS_AttributeWidgetController* AGAS_HUD::GetAttributeWidgetController(const FGAS_WidgetControllerParams& WCParams)
{
	if (AttributeWidgetController == nullptr)
	{
		AttributeWidgetController = NewObject<UGAS_AttributeWidgetController>(this,AttributeWidgetControllerClass);
		AttributeWidgetController->SetWidgetControllerParams(WCParams);
		AttributeWidgetController->BindCallbacksToDependencies();
	}
	return AttributeWidgetController;
}
