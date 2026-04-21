#include "AbilityComponent/GAS_FunctionLibrary.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "GAS_GameplayTags.h"
#include "Data/CharacterClassInfo.h"
#include "ScalableFloat.h"
#include "Data/GAS_AbilityTypes.h"
#include "Game/GAS_GameModeBase.h"
#include "Kismet/GameplayStatics.h"

UCharacterClassInfo* UGAS_FunctionLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	return nullptr;
	//const AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	//if (AuraGameMode == nullptr) return nullptr;
	//return AuraGameMode->CharacterClassInfo;
}

int32 UGAS_FunctionLibrary::GetXPRewardForClassAndLevel(const UObject* WorldContextObject,
                                                        ECharacterClass CharacterClass, int32 CharacterLevel)
{
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr) return 0;

	const FCharacterClassDefaultInfo& Info = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
	const float XPReward = Info.XPReward.GetValueAtLevel(CharacterLevel);

	return static_cast<int32>(XPReward);
}

void UGAS_FunctionLibrary::SetDeathImpulse(FGameplayEffectContextHandle& EffectContexthandle,
                                                  const FVector& InImpulse)
{
	if (FGAS_GameplayEffectContext* EffectContext = static_cast<FGAS_GameplayEffectContext*>(EffectContexthandle.Get()))
	{
		EffectContext->SetDeathImpulse(InImpulse);
	}
}

void UGAS_FunctionLibrary::SetKnockbackForce(FGameplayEffectContextHandle& EffectContexthandle,
	const FVector& InImpulse)
{
	if (FGAS_GameplayEffectContext* EffectContext = static_cast<FGAS_GameplayEffectContext*>(EffectContexthandle.Get()))
	{
		EffectContext->SetKnockbackForce(InImpulse);
	}
}

FGameplayEffectContextHandle UGAS_FunctionLibrary::ApplyDamageEffect(
	const FDamageEffectParams& DamageEffectParams)
{
	const AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor();
	
	FGameplayEffectContextHandle EffectContexthandle = DamageEffectParams.SourceAbilitySystemComponent->MakeEffectContext();
	EffectContexthandle.AddSourceObject(SourceAvatarActor);
	SetDeathImpulse(EffectContexthandle, DamageEffectParams.DeathImpulse);
	SetKnockbackForce(EffectContexthandle, DamageEffectParams.KnockbackForce);

	const FGameplayEffectSpecHandle SpecHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeOutgoingSpec(DamageEffectParams.DamageGameplayEffectClass, DamageEffectParams.AbilityLevel, EffectContexthandle);

	AssignTagSetByCallerMagnitude(SpecHandle, DamageEffectParams.DamageType, DamageEffectParams.BaseDamage);
	

	DamageEffectParams.TargetAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
	return EffectContexthandle;
}

FGameplayEffectSpecHandle UGAS_FunctionLibrary::AssignTagSetByCallerMagnitude(
	FGameplayEffectSpecHandle SpecHandle, FGameplayTag DataTag, float Magnitude)
{
	FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
	if (Spec)
	{
		Spec->SetSetByCallerMagnitude(DataTag, Magnitude);
	}
	else
	{
		UE_LOG(LogTemp,Warning, TEXT("USystemBlueprintLibrary::AssignSetByCallerTagMagnitude called with invalid SpecHandle"));
	}

	return SpecHandle;
}

bool UGAS_FunctionLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FGAS_GameplayEffectContext* AuraEffectContext = static_cast<const FGAS_GameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->IsCriticalHit();
	}
	return false;
}

void UGAS_FunctionLibrary::SetIsCriticalHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsCriticalHit)
{
	if (FGAS_GameplayEffectContext* AuraEffectContext = static_cast<FGAS_GameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetIsCriticalHit(bInIsCriticalHit);
	}
}
