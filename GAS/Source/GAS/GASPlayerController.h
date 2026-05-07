// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Data/AbilityInputInfo.h"
#include "GASPlayerController.generated.h"

struct FGameplayTag;
class UGAS_AbilitySystemComponent;
class UInputMappingContext;
class UUserWidget;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEnemyDetected,bool);

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings
 */
UCLASS(abstract)
class AGASPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

public:

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void DisplayDamageText(float IncomingDamage,const FVector& ActorLocation);

	FOnEnemyDetected OnEnemyDetected;
private:

	UPROPERTY()
	TObjectPtr<UGAS_AbilitySystemComponent> AuroraAbilitySystemComponent;

	UGAS_AbilitySystemComponent* GetASC();

	UPROPERTY(EditAnywhere)
	UAbilityInputInfo* InputConfig;
		
	void AbilityInputPressed(const FGameplayTag Tag);
	UFUNCTION()
	void AbilityInputReleased(const FGameplayTag Tag);
	UFUNCTION()
	void AbilityInputHeld(const FGameplayTag Tag);

	///
	// Detection Trace
	///

	// The length of the detection trace in units 100 unit = 1meter
	UPROPERTY(EditDefaultsOnly,Category="Detection Trace")
	float DetectionTraceLength = 400.f;

	/**
	* Time interval in seconds between each enemy detection line trace.
	* Lower values update target detection more frequently but are more expensive.
	*/
	UPROPERTY(EditDefaultsOnly,Category="Detection Trace")
	float EnemyTraceInterval = 0.1f;
	
	FTimerHandle TraceTimerHandle;
	void PerformEnemyTrace();
};
