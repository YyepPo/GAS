#include "AbilityComponent/GAS_FunctionLibrary.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "GAS_GameplayTags.h"
#include "Data/CharacterClassInfo.h"
#include "ScalableFloat.h"
#include "Data/GAS_AbilityTypes.h"
#include "Game/GAS_GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/ArchiveSerializedPropertyChain.h"

UCharacterClassInfo* UGAS_FunctionLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	const AGAS_GameModeBase* GameMode = Cast<AGAS_GameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (GameMode == nullptr) return nullptr;
	return GameMode->CharacterClassInfo;
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

	if (SpecHandle.Data.IsValid())
	{
		DamageEffectParams.TargetAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
		return EffectContexthandle;	
	}
	return FGameplayEffectContextHandle();
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

void UGAS_FunctionLibrary::InitializeDefaultAttributes(const UObject* WorldContextObject,
	ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC)
{
	AActor* AvatarActor = ASC->GetAvatarActor();

	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	FCharacterClassDefaultInfo ClassDefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);

	FGameplayEffectContextHandle PrimaryAttributesContextHandle = ASC->MakeEffectContext();
	PrimaryAttributesContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle PrimaryAttributesSpecHandle = ASC->MakeOutgoingSpec(ClassDefaultInfo.PrimaryAttributes, Level, PrimaryAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*PrimaryAttributesSpecHandle.Data.Get());

	FGameplayEffectContextHandle SecondaryAttributesContextHandle = ASC->MakeEffectContext();
	SecondaryAttributesContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle SecondaryAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->SecondaryAttributes, Level, SecondaryAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*SecondaryAttributesSpecHandle.Data.Get());

	FGameplayEffectContextHandle VitalAttributesContextHandle = ASC->MakeEffectContext();
	VitalAttributesContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle VitalAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->VitalAttributes, Level, VitalAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*VitalAttributesSpecHandle.Data.Get());
}

FVector UGAS_FunctionLibrary::GetKnockbackForce(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FGAS_GameplayEffectContext* AuraEffectContext = static_cast<const FGAS_GameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->GetKnockbackForce();
	}
	return FVector::ZeroVector;
}

FVector UGAS_FunctionLibrary::GetDeathImpulse(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FGAS_GameplayEffectContext* AuraEffectContext = static_cast<const FGAS_GameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->GetDeathImpulse();
	}
	return FVector::ZeroVector;
}

bool UGAS_FunctionLibrary::ApplyBlockMovementTag(UAbilitySystemComponent* ASC)
{
	if (ASC == nullptr)
	{
		return false;
	}
	
	FGAS_GameplayTags::Get();
	const FGAS_GameplayTags& GameplayTags = FGAS_GameplayTags::Get();
	FGameplayTagContainer BlockedTags;
	BlockedTags.AddTag(GameplayTags.Player_Block_InputHeld);
	BlockedTags.AddTag(GameplayTags.Player_Block_InputPressed);
	BlockedTags.AddTag(GameplayTags.Player_Block_InputReleased);
	
	ASC->AddLooseGameplayTags(BlockedTags);

	return true;
}

bool UGAS_FunctionLibrary::RemoveBlockMovementTag(UAbilitySystemComponent* ASC)
{
	if (ASC == nullptr)
	{
		return false;
	}
	
	FGAS_GameplayTags::Get();
	const FGAS_GameplayTags& GameplayTags = FGAS_GameplayTags::Get();
	FGameplayTagContainer BlockedTags;
	BlockedTags.AddTag(GameplayTags.Player_Block_InputHeld);
	BlockedTags.AddTag(GameplayTags.Player_Block_InputPressed);
	BlockedTags.AddTag(GameplayTags.Player_Block_InputReleased);
	
	ASC->RemoveLooseGameplayTags(BlockedTags);

	return true;
}
