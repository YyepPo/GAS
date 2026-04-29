#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GAS_HUD.generated.h"

class UGAS_AttributeWidgetController;
class UAttributeSet;
class UOverlayWidget;
class UGAS_OverlayWidgetController;
class UAbilitySystemComponent;
struct FGAS_WidgetControllerParams;

UCLASS()
class GAS_API AGAS_HUD : public AHUD
{
	GENERATED_BODY()

public:
	
	void InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);
	
	UGAS_OverlayWidgetController* GetOverlayWidgetController(const FGAS_WidgetControllerParams& WCParams);
	UGAS_AttributeWidgetController* GetAttributeWidgetController(const FGAS_WidgetControllerParams& WCParams);
private:

	UPROPERTY()
	TObjectPtr<UOverlayWidget>  OverlayWidget;
	UPROPERTY(EditAnywhere)
	TSubclassOf<UOverlayWidget>  OverlayWidgetClass;

	UPROPERTY()
	TObjectPtr<UGAS_OverlayWidgetController> OverlayWidgetController;
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGAS_OverlayWidgetController> OverlayWidgetControllerClass;

	UPROPERTY()
	TObjectPtr<UGAS_AttributeWidgetController> AttributeWidgetController;
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGAS_AttributeWidgetController> AttributeWidgetControllerClass;
};
