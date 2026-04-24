
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS_BaseCharacter.h"
#include "Interface/CharacterInterface.h"
#include "GAS_AuroraCharacter.generated.h"

UCLASS()
class GAS_API AGAS_AuroraCharacter : public AGAS_BaseCharacter, public ICharacterInterface
{
	GENERATED_BODY()

public:

	AGAS_AuroraCharacter();

protected:

	virtual void BeginPlay() override;
	
	/** Called from server, initializes AbilitySystemComponent on the server */
	virtual void PossessedBy(AController* NewController) override;

	/** Called on client, initializes AbilitySystemComponent on the client */
	virtual void OnRep_PlayerState() override;
	
	
	/* IPlayerInterface Functions **/
	virtual int32 FindLevelForXP_Implementation(int32 InXP) const override;	
	virtual int32 GetXP_Implementation() const override;
	virtual int32 GetAttributePointsReward_Implementation(int32 Level) const override;
	virtual int32 GetSpellPointsReward_Implementation(int32 Level) const override;
	virtual void AddToXP_Implementation(int32 InXP) override;
	virtual void AddToPlayerLevel_Implementation(int32 InPlayerLevel) override;
	virtual void AddToAttributePoints_Implementation(int32 InAttributePoints) override;
	virtual int32 GetAttributePoints_Implementation() const override;
	virtual void AddToSpellPoints_Implementation(int32 InSpellPoints) override;
	virtual int32 GetSpellPoints_Implementation() const override;
	virtual void LevelUp_Implementation() override;
	virtual void ShowHitMarker_Implementation() override;
	
	/* IPlayerInterface Functions  END **/

	virtual void InitAbilityInfo() override;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	/** Called for movement input */
	UFUNCTION()
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	UFUNCTION()
	void Look(const FInputActionValue& Value);

	UPROPERTY(EditAnywhere, Category = "Camera Shake")
	TSubclassOf<class UCameraShakeBase> HitCameraShake;
	
public:

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
