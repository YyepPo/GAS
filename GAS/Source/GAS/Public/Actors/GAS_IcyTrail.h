// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GAS_IcyTrail.generated.h"

class UGameplayEffect;

UCLASS()
class GAS_API AGAS_IcyTrail : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGAS_IcyTrail();

protected:
	
	virtual void BeginPlay() override;

public:

private:
	
	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* OverlapBox;
};
