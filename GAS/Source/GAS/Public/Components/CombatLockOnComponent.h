// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatLockOnComponent.generated.h"

class AGAS_AuroraCharacter;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLockTargetUpdated, AActor*, Target);

class UCameraComponent;
enum class ELockOnSwitchDirection : uint8
{
	Left,
	Right
};

/**
 * UCombatLockOnComponent handles the logic for a combat targeting system (Lock-On).
 * It detects nearby lockable actors within a specified radius using sphere overlap,
 * determines if they are in front of the camera and visible (not blocked by obstacles),
 * and locks onto the best valid target. It also supports switching between targets
 * based on screen space projection.
 */
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

	
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	/** 
	 * Initiates the lock-on process. Scans for valid targets around the owner actor 
	 * and locks onto the first visible target in front of the camera.
	 * @param CameraComponent The camera used to determine forward direction and visibility line traces.
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock")
	void StartLock(UCameraComponent* CameraComponent);
	
	/** 
	 * Ends the current lock-on, clears targets, and broadcasts that the target has been cleared.
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock")
	void StopLock();

	/** Evaluates valid targets and switches to a new one based on screen space proximity. */
	UFUNCTION(BlueprintCallable, Category = "Lock")
	void SwitchTarget();

	/** Switches to the nearest valid target on the left side of the current target in screen space. */
	UFUNCTION(BlueprintCallable, Category = "Lock")
	void SwitchTargetLeft();

	/** Switches to the nearest valid target on the right side of the current target in screen space. */
	UFUNCTION(BlueprintCallable, Category = "Lock")
	void SwitchTargetRight();
	
	/** Delegate fired when the locked target changes. Broadcasts the new target (or nullptr if lock stopped). */
	UPROPERTY(Blueprintassignable)
	FOnLockTargetUpdated OnLockTargetUpdated;
	
	/** Indicates whether a lock-on is currently active. */
	bool bLockStarted = false;
	bool bIsRestoringFromLock;
	FRotator PreLockSpringArmRotation;
	FVector PreLockSpringArmLocation;
	FRotator PreLockActorRotation;

	/// 
	// Getters
	///  

	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "Lock")
	FORCEINLINE AActor* GetCurrentTarget() const {return CurrentTarget;}
	
private:

	///
	// Overlap detection properties
	///
	
	/** Radius used for the sphere overlap to detect lockable targets. */
	UPROPERTY(EditAnywhere, Category = "Overlap")
	float SphereRadius = 300.0f;

	/** Object types to look for when scanning for targets. */
	UPROPERTY(EditAnywhere, Category = "Overlap")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;

	/** Actors that should be ignored during the overlap scan (usually includes the owner). */
	UPROPERTY(EditAnywhere, Category = "Overlap")
	TArray<AActor*> ActorsToIgnore;

	/** Specific class filter for targets to be considered valid lock-on candidates. */
	UPROPERTY(EditAnywhere, Category = "Overlap")
	TSubclassOf<AActor> ActorClassFilter;
	
	
	/* List of targets that can be locked in **/
	UPROPERTY()
	TArray<AActor*> LockableTargets;	
	
	/* Current target that we are locked in **/
	UPROPERTY()
	AActor* CurrentTarget;
	
	/* Cached camera component of the player. Used as the start point of detection line trace **/
	UPROPERTY()
	UCameraComponent* CachedCamera;

	/** Checks if the target is in the general forward direction of the camera. */
	bool IsTargetInFront(AActor* Target,const FVector& CameraLocation,const FVector& CameraForwardVector);

	/** Performs a line trace to check if there is a clear line of sight to the target. */
	bool IsTargetVisible(AActor* Target, const FVector& CameraLocation);

	/** Sets the current target and broadcasts the update delegate. */
	void LockToTarget(AActor* Target);

	/** Selects a target that is closest to the middle of the screen */
	AActor* SelectTargetClosestToMiddleOfTheScreen(const TArray<AActor*>& Actors);

	/** Selects the next valid target relative to the current target in screen space. */
	AActor* SelectNextTarget(const TArray<AActor*>& Actors, ELockOnSwitchDirection Direction) const;

	/** Projects a valid target into screen space and rejects dead, off-screen, or behind-camera targets. */
	bool TryProjectTargetToScreen(AActor* Target, FVector2D& OutScreenPosition) const;

	/** Returns whether the target can currently be used by the lock-on system. */
	bool IsTargetAlive(AActor* Target) const;
	
	TArray<AActor*> GetLockableTargets();
	
	void ListenForTargetDeathEvent(AActor* Target);
	UFUNCTION()
	void OnTargetDeathEvent(AActor* Target);
		
	/** Returns whether there are any lockable targets available. */
	bool CanLock() const;

	/** Helper to get the component's owner. */
	AActor* GetOwnerActor() const;

	AGAS_AuroraCharacter* GetOwnerCharacter() const;

	/** Helper to retrieve the player controller of the owner pawn. */
	APlayerController* GetPlayerController() const;
	
	int32 LockCounter = 0;

	UPROPERTY()
	class USpringArmComponent* CachedSpringArm;

	UPROPERTY(EditAnywhere)
	FVector SpringArmOffsetOnLock;

	UPROPERTY(EditAnywhere)
	float SpringRotateInterpSpeed = 10.f;
	UPROPERTY(EditAnywhere)
	float SpringLocationInterpSpeed = 10.f;
	UPROPERTY(EditAnywhere)
	float MeshRotateInterSpeed = 10.f;

	UPROPERTY(EditAnywhere, Category = "Lock")
	bool bWrapSwitchTarget = true;

	void SavePreLockState();
};
