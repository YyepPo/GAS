#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "GAS_IceRainSpike.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;
class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class GAS_API AGAS_IceRainSpike : public AActor
{
	GENERATED_BODY()

public:
	AGAS_IceRainSpike();

	void InitializeSpike(
		UAbilitySystemComponent* InSourceAbilitySystemComponent,
		TSubclassOf<UGameplayEffect> InDamageEffectClass,
		TSubclassOf<UGameplayEffect> InStunEffectClass,
		float InAbilityLevel,
		float InBaseDamage,
		FGameplayTag InDamageType,
		const FVector& InVelocity);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ice Rain|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ice Rain|Damage")
	TSubclassOf<UGameplayEffect> StunEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ice Rain|Damage", meta = (ClampMin = "0.0"))
	float BaseDamage = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ice Rain|Damage", meta = (Categories = "Damage"))
	FGameplayTag DamageType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ice Rain|Movement", meta = (ClampMin = "0.0", Units = "s"))
	float Lifetime = 3.f;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> SpikeMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UFUNCTION()
	void OnSpikeHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

	void ApplyEffectsToTarget(AActor* TargetActor, const FHitResult& Hit);

	float AbilityLevel = 1.f;
	bool bHasImpacted = false;
};
