// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS_BaseCharacter.h"
#include "GAS_Enemy.generated.h"

UCLASS()
class GAS_API AGAS_Enemy : public AGAS_BaseCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGAS_Enemy();

	virtual int32 GetPlayerLevel_Implementation() override;


protected:

	virtual void BeginPlay() override;

	virtual void InitAbilityInfo() override;
	virtual void ApplyDefaultAttributes() const override;
	
private:

	int32 Level = 1;
	
};
