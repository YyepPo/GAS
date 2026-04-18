// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "GAS_AssetManager.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGAS_AssetManager : public UAssetManager
{
	GENERATED_BODY()
	
public:
	
	static UGAS_AssetManager& Get();
	
protected:
	
	virtual void StartInitialLoading() override;
};
