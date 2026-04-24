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

void AGAS_IcyTrail::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void AGAS_IcyTrail::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}
