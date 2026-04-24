#include "Actors/FrostRing.h"

#include "Actors/IceSpike.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values
AFrostRing::AFrostRing()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AFrostRing::BeginPlay()
{
	Super::BeginPlay();

	SpawnIceSpikes();
}

void AFrostRing::SpawnIceSpikes()
{
	if (!IceSpikeClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("AFrostRing: IceSpikeClass is not set"));
		return;
	}

	TArray<FVector> Locations;

	// First loop — calculate all spawn locations
	for (int32 i = 0; i < SpikeCount; i++)
	{
		const float Angle = (360.f / SpikeCount) * i;
		const float RadAngle = FMath::DegreesToRadians(Angle);

		FVector SpawnOffset = FVector(
			FMath::Cos(RadAngle) * Radius,
			FMath::Sin(RadAngle) * Radius,
			0.f
		);

		Locations.Add(GetActorLocation() + SpawnOffset);
	}

	// Second loop — spawn spikes with correct rotation and scale
	for (int32 i = 0; i < SpikeCount; i++)
	{
		// Look at next spike, wraps back to 0 on last spike
		const int32 NextIndex = (i + 1) % SpikeCount;
		const FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(
			Locations[i], Locations[NextIndex]);

		// Scale grows from 0.5 to ~2.5 across all spikes
		const float EmitterScale = FMath::Lerp(0.5f, 1.5, (float)i / SpikeCount);

		FTransform SpawnTransform;
		SpawnTransform.SetLocation(Locations[i]);
		SpawnTransform.SetRotation(Rotation.Quaternion());
		SpawnTransform.SetScale3D(FVector(EmitterScale));
		
		AIceSpike* Spike = GetWorld()->SpawnActor<AIceSpike>(
			IceSpikeClass,
			SpawnTransform
		);

		if (Spike)
		{
			Spike->SetInstigator(GetInstigator());
			Spike->SetAbilityLevel(1);
		}
	}
}