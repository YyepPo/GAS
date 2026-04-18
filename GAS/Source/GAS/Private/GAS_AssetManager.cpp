// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS_AssetManager.h"

#include "GAS_GameplayTags.h"

UGAS_AssetManager& UGAS_AssetManager::Get()
{
	check(GEngine);
	
	UGAS_AssetManager* GASAssetManager = Cast<UGAS_AssetManager>(GEngine->AssetManager);
	return *GASAssetManager;
}

void UGAS_AssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	FGAS_GameplayTags::InitializeNativeGameplayTags();
	//UAbilitySystemGlobal
}

