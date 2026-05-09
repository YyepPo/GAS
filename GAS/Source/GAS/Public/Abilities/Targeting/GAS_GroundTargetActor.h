#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "GAS_GroundTargetActor.generated.h"

class UDecalComponent;
class UMaterialInterface;
class USceneComponent;

UCLASS()
class GAS_API AGAS_GroundTargetActor : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	AGAS_GroundTargetActor();

	virtual void StartTargeting(UGameplayAbility* Ability) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual bool IsConfirmTargetingAllowed() override;
	virtual void ConfirmTargetingAndContinue() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
	float TargetRadius = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
	float MaxTraceDistance = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
	float MinSurfaceNormalZ = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting|Visual")
	TObjectPtr<UMaterialInterface> DecalMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting|Visual")
	float DecalDepth = 32.f;

protected:
	bool UpdateTargetingTrace();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UDecalComponent> TargetDecal;

	FHitResult CurrentHitResult;
	TWeakObjectPtr<APlayerController> CachedPlayerController;
	bool bHasValidTarget = false;
};
