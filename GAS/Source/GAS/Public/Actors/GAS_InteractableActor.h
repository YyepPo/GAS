#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/IInteractableTarget.h"
#include "GAS_InteractableActor.generated.h"

class UStaticMeshComponent;

UCLASS(Blueprintable)
class GAS_API AGAS_InteractableActor : public AActor, public IIInteractableTarget
{
	GENERATED_BODY()

public:
	AGAS_InteractableActor();

	virtual void GatherInteractionOptions(const FInteractionQueryResult& InteractionQueryResult, FInteractionOptionBuilder& InteractionOptionBuilder) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	FInteractionOption InteractionOption;
};
