#include "Actors/GAS_InteractableActor.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "Components/StaticMeshComponent.h"
#include "GameplayTagContainer.h"

AGAS_InteractableActor::AGAS_InteractableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);

	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Overlap);
}

void AGAS_InteractableActor::GatherInteractionOptions(const FInteractionQueryResult& InteractionQueryResult, FInteractionOptionBuilder& InteractionOptionBuilder)
{
	InteractionOptionBuilder.AddInteractionOption(InteractionOption);
}
