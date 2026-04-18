#pragma once

#include "CoreMinimal.h"
#include "Data/CharacterClassInfo.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GAS_FunctionLibrary.generated.h"

UCLASS()
class GAS_API UGAS_FunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static UCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContextObject);
	static int32 GetXPRewardForClassAndLevel(const UObject* WorldContextObject,
	                                         ECharacterClass CharacterClass,
	                                         int32 CharacterLevel);
	
	
};
