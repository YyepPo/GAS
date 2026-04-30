#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GAS_AbilitySystemLibrary.generated.h"

class UGAS_OverlayWidgetController;
class UGAS_AttributeWidgetController;

UCLASS()
class GAS_API UGAS_AbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintCallable, Category = "GAS|Combat")
	static FGameplayTag CalculateHitDirection(AActor* SourceCharacter,AActor* TargetActor);

	UFUNCTION(BlueprintCallable)
	static float GetDirectionToTargetInDegress(const FVector& ActorForwardVector,const FVector& DirectionToTarget);

	UFUNCTION(BlueprintCallable)
	static UGAS_OverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable)
	static UGAS_AttributeWidgetController* GetAttributeWidgetController(const UObject* WorldContextObject);
};
