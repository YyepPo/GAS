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
	
	// Allies move faster on trail
	UPROPERTY(EditDefaultsOnly, Category = "Trail")
	TSubclassOf<UGameplayEffect> AllySpeedBoostEffectClass;

	// How long trail lasts before auto destroy
	UPROPERTY(EditDefaultsOnly, Category = "Trail")
	float TrailLifetime = 10.f;

	void DestroyTrail();

private:
	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* OverlapBox;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* TrailVFX;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Track active speed boosts to remove on exit
	//UPROPERTY()
	//TMap<AActor*, FActiveGameplayEffectHandle> ActiveBoosts;
};
