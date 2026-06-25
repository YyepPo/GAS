// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LockInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class ULockInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GAS_API ILockInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent,BlueprintCallable,Category = "LockOn")
	FVector GetLockOnLocation() const;
};
