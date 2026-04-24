// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IceSpike.generated.h"

class UGameplayEffect;

UCLASS()
class GAS_API AIceSpike : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AIceSpike();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> FreezeEffectClass;

	UPROPERTY(EditDefaultsOnly)
	float Lifetime = 3.f;

	void SetAbilityLevel(float Level) { AbilityLevel = Level; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	float AbilityLevel = 1.f;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* SpikeMesh;

	UPROPERTY(VisibleAnywhere)
	class USphereComponent* CollisionSphere;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

 
};
