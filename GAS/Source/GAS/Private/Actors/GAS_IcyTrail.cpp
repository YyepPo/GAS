#include "Actors/GAS_IcyTrail.h"
#include "Components/BoxComponent.h"

AGAS_IcyTrail::AGAS_IcyTrail()
{
	PrimaryActorTick.bCanEverTick = false;
	
	OverlapBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Overlap Box"));
	SetRootComponent(OverlapBox);

}

// Called when the game starts or when spawned
void AGAS_IcyTrail::BeginPlay()
{
	Super::BeginPlay();
	
}
