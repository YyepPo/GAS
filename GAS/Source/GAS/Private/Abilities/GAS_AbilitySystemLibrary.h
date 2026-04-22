#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GAS_AbilitySystemLibrary.generated.h"

UCLASS()
class GAS_API UGAS_AbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintCallable, Category = "GAS|Combat")
	static FGameplayTag CalculateHitDirection(AActor* SourceCharacter,AActor* TargetActor);

	UFUNCTION(BlueprintCallable)
	static float GetDirectionToTargetInDegress(const FVector& ActorForwardVector,const FVector& DirectionToTarget);
};
