#pragma once

#include "CoreMinimal.h"
#include "InteractionQuery.generated.h"

USTRUCT(BlueprintType)
struct FInteractionQueryResult
{
    GENERATED_BODY();

    /* The avatar actor that wants to interact with the item **/
    UPROPERTY(BlueprintReadWrite)
    TWeakObjectPtr<AActor> InteractingActor;

    /*  The controller that wants to interact with the item **/
    UPROPERTY(BlueprintReadWrite)
    TWeakObjectPtr<AController> InteractingController;

    /* Additional custom data about the item **/
    UPROPERTY(BlueprintReadWrite)
    TWeakObjectPtr<UObject> ExtraInfo;
};