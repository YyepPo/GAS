#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GAS_GlacialChargeIceBlock.generated.h"

class UBoxComponent;
class USceneComponent;
class UStaticMeshComponent;
class AActor;

UCLASS()
class GAS_API AGAS_GlacialChargeIceBlock : public AActor
{
	GENERATED_BODY()

public:
	AGAS_GlacialChargeIceBlock();

	void InitializeIceBlock(const FVector& InDimensions, AActor* InIgnoredActor = nullptr);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Charge")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Charge")
	TObjectPtr<UStaticMeshComponent> IceMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Charge")
	TObjectPtr<UBoxComponent> CollisionVolume;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Charge")
	float LifeSeconds = 6.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Charge")
	float CollisionPadding = 1.f;

private:
	void ConfigureCollisionForDashOwner() const;

	FVector BlockDimensions = FVector(100.f, 10.f, 10.f);
	TWeakObjectPtr<AActor> IgnoredActor;
};
