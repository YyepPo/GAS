// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FrostRing.generated.h"

class AIceSpike;

UCLASS()
class GAS_API AFrostRing : public AActor
{
	GENERATED_BODY()

public:
	
	AFrostRing();

protected:
	
	virtual void BeginPlay() override;

private:

	void SpawnIceSpikes();

	UPROPERTY(EditAnywhere)
	TSubclassOf<AIceSpike> IceSpikeClass;

	UPROPERTY(EditAnywhere)
	int32 SpikeCount = 8;
	
	UPROPERTY(EditAnywhere)
	float Radius = 150.f;
};
