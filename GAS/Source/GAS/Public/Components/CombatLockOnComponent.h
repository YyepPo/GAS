// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatLockOnComponent.generated.h"


class UCameraComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAS_API UCombatLockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCombatLockOnComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Lock")
	void StartLock(UCameraComponent* CameraComponent);
	UFUNCTION(BlueprintCallable, Category = "Lock")
	void StopLock();
	
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
private:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap")
	float SphereRadius = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap")
	TArray<AActor*> ActorsToIgnore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap")
	TSubclassOf<AActor> ActorClassFilter;
	
	/* List of targets that can be locked in **/
	UPROPERTY()
	TArray<AActor*> LockableTargets;	
	
	/* Current target that we are locked in **/
	UPROPERTY()
	AActor* CurrentTarget;
	
	UPROPERTY()
	UCameraComponent* CachedCamera;
	
	bool bLockStarted = false;
	
	bool IsTargetInFront(AActor* Target,const FVector& CameraLocation,const FVector& CameraForwardVector);
	bool IsTargetVisible(AActor* Target, const FVector& CameraLocation);
	void LockToTarget(AActor* Target);
	void SwitchTarget();
	
	bool CanLock() const;
	
	AActor* GetOwnerActor() const;
	APlayerController* GetPlayerController() const;
	
	int32 CurrentTargetIndex = 0;
	
};
