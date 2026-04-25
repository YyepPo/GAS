#pragma once

#include "CoreMinimal.h"
#include "GAS_BaseAbility.h"
#include "GAS_GlacialCharge.generated.h"

class AGAS_IcyTrail;

UCLASS()
class GAS_API UGAS_GlacialCharge : public UGAS_BaseAbility
{
	GENERATED_BODY()
	
	UGAS_GlacialCharge();
	
public:

	UFUNCTION(BlueprintCallable)
	void SpawnTrailSegment();
	
private:
	
	// GA_GlacialCharge.h
	UPROPERTY(EditDefaultsOnly, Category = "Charge")
	float ChargeSpeed = 1800.f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge")
	float ChargeDuration = 2.f;

	UPROPERTY(EditDefaultsOnly, Category = "Trail")
	float TrailSpawnInterval = 0.15f;

	UPROPERTY(EditDefaultsOnly, Category = "Trail")
	TSubclassOf<AGAS_IcyTrail> IcyTrailClass;

	UPROPERTY(EditDefaultsOnly, Category = "Trail")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Trail")
	TSubclassOf<UGameplayEffect> PushEffectClass;

	FTimerHandle TrailSpawnTimer;
	FTimerHandle DamageTimer;
	FTimerHandle ChargeDurationTimer;
	
	UFUNCTION()
	void OnMontageCompleted();
	UFUNCTION()
	void PerformChargeSweep();
	
	UPROPERTY()
	TArray<AGAS_IcyTrail*> SpawnedTrailSegments;
	bool bIsCharging = false;
	
	UPROPERTY(EditDefaultsOnly, Category = "Charge")
	float PushForce = 800.f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge")
	float PushZForce = 300.f;
	
	UPROPERTY(EditAnywhere)
	UAnimMontage* ChargeMontage;
};
