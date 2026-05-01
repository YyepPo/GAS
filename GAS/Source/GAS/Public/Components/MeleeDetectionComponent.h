// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MeleeDetectionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnHitDetected, const FHitResult&, HitResult);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAS_API UMeleeDetectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	// Sets default values for this component's properties
	UMeleeDetectionComponent();

protected:
	
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BlueprintCallable)
	void StartTrace(const FName InStartSocket,const FName InEndSocket);
	
	UFUNCTION(BlueprintCallable)
	void StopTrace();
	
	UPROPERTY(BlueprintAssignable)
	FOnHitDetected OnHitDetected;
	
private:
	
	/* Whether the trace is happening or not. Is being checked in tick **/
	bool bStartTrace = false;
	
	/* The point where trace starts **/
	FName TraceStartSocket;
	/* The point where trace ends **/
	FName TraceEndSocket;
	
	/* Detects whether a trace/sweep hits something
	 * Does the tracing logic inside the tick **/
	void DoTrace();
	
	/* A list of hit actors. Used to prevent hitting an actor twice **/
	UPROPERTY()
	TSet<TObjectPtr<AActor>> HitActors;
	
	// Store previous frame positions
	FVector PreviousStartLocation;
	FVector PreviousEndLocation;
};
