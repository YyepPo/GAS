// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MeleeDetectionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnHitDetected, const FHitResult&, HitResult);

/**
 * UMeleeDetectionComponent handles precise hit detection for melee combat.
 * It traces collision shapes (like capsules) between specified sockets 
 * on a skeletal mesh over time to accurately detect hits, even at low frame rates.
 * It caches previous frame locations to perform continuous collision detection (sweep)
 * preventing fast-moving attacks from passing through targets.
 */
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
	
	/** 
	 * Initiates the melee trace detection. Called usually at the start of an attack animation via AnimNotify.
	 * @param InStartSocket The starting socket name on the skeletal mesh (e.g., weapon base).
	 * @param InEndSocket The ending socket name on the skeletal mesh (e.g., weapon tip).
	 * @param EnableDebug If true, draws debug shapes to visualize the trace.
	 * @param InCapsuleRadius The radius of the sweep shape used for detection.
	 * @param InCapsuleHeight The half-height of the sweep shape used for detection.
	 */
	UFUNCTION(BlueprintCallable)
	void StartTrace(const FName InStartSocket,const FName InEndSocket,
		bool EnableDebug,float InCapsuleRadius,float InCapsuleHeight);
	
	/** 
	 * Stops the melee trace detection. Called usually at the end of an attack animation.
	 * Clears the list of hit actors, allowing them to be hit again in subsequent attacks.
	 */
	UFUNCTION(BlueprintCallable)
	void StopTrace();
	
	/** Delegate fired whenever the melee trace detects a valid hit. */
	UPROPERTY(BlueprintAssignable)
	FOnHitDetected OnHitDetected;
	
private:
	
	/** Whether the trace is currently active. Checked every frame in TickComponent. */
	bool bStartTrace = false;
	
	/** The socket name where the trace segment begins. */
	FName TraceStartSocket;
	
	/** The socket name where the trace segment ends. */
	FName TraceEndSocket;
	
	/** 
	 * Performs the actual tracing logic. Uses a sweep from the previous frame's socket 
	 * locations to the current frame's socket locations to ensure robust collision 
	 * detection regardless of animation speed or frame rate.
	 */
	void DoTrace();
	
	/** A list of actors already hit during the current trace. Used to prevent hitting the same actor multiple times in one swing. */
	UPROPERTY()
	TSet<TObjectPtr<AActor>> HitActors;
	
	// The reason of doing this is that in low frame rates there is a huge gap btw each detection
	// Storing previous positions allows sweeping between frames for continuous collision detection.
	
	/** Cached starting socket location from the previous frame. */
	FVector PreviousStartLocation;
	
	/** Cached ending socket location from the previous frame. */
	FVector PreviousEndLocation;
	
	/** Whether to draw debug visuals for the collision traces. */
	UPROPERTY(EditDefaultsOnly)
	bool bEnableDebug = false;
	
	/** Radius of the capsule used for the sweep trace. */
	float CapsuleRadius;
	
	/** Half-height of the capsule used for the sweep trace. */
	float CapsuleHalfHeight;
	
	/** Helper function to retrieve the skeletal mesh component of the owner actor. */
	USkeletalMeshComponent* GetOwnersMesh() const;
};
