#include "AbilityComponent/GAS_FunctionLibrary.h"
#include "Data/CharacterClassInfo.h"
#include "ScalableFloat.h"
#include "Game/GAS_GameModeBase.h"
#include "Kismet/GameplayStatics.h"

int32 UGAS_FunctionLibrary::GetXPRewardForClassAndLevel(const UObject* WorldContextObject,
                                                        ECharacterClass CharacterClass, int32 CharacterLevel)
{
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr) return 0;

	const FCharacterClassDefaultInfo& Info = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
	const float XPReward = Info.XPReward.GetValueAtLevel(CharacterLevel);

	return static_cast<int32>(XPReward);
}

UCharacterClassInfo* UGAS_FunctionLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	const AGAS_GameModeBase* GASGameMode = Cast<AGAS_GameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (GASGameMode == nullptr) return nullptr;
	return GASGameMode->CharacterClassInfo;
}