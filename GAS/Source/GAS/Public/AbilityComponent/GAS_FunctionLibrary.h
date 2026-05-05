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

	UFUNCTION(BlueprintCallable, Category = "GameplayEffect", meta = (GameplayTagFilter = "SetByCaller"))
	static FGameplayEffectSpecHandle AssignTagSetByCallerMagnitude(FGameplayEffectSpecHandle SpecHandle, FGameplayTag DataTag, float Magnitude);
	
	UFUNCTION(BlueprintCallable, Category = "DamageEffect")
	static void SetDeathImpulse(FGameplayEffectContextHandle& EffectContexthandle, const FVector& InImpulse);
	
	UFUNCTION(BlueprintCallable, Category = "DamageEffect")
	static void SetKnockbackForce(FGameplayEffectContextHandle& EffectContexthandle, const FVector& InImpulse);
	
	UFUNCTION(BlueprintCallable, Category = "DamageEffect")
	static FGameplayEffectContextHandle ApplyDamageEffect(const FDamageEffectParams& DamageEffectParams);

	UFUNCTION(BlueprintCallable, Category = "DamageEffect")
	static bool IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintCallable, Category = "GameplayEffects")
	static void SetIsCriticalHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsCriticalHit);

	UFUNCTION(BlueprintCallable, Category = "AbilitySystemLibrary")
	static void InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC);

	UFUNCTION(BlueprintPure, Category = "GameplayEffects")
	static FVector GetKnockbackForce(const FGameplayEffectContextHandle& EffectContextHandle);
	
	UFUNCTION(BlueprintPure, Category = "GameplayEffects")
	static FVector GetDeathImpulse(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintCallable, Category = "Tag")
	static bool ApplyBlockMovementTag(UAbilitySystemComponent* ASC);

	UFUNCTION(BlueprintCallable, Category = "Tag")
	static bool RemoveBlockMovementTag(UAbilitySystemComponent* ASC);

	UFUNCTION(BlueprintCallable,Category = "Event")
	static void SendRegenEvent(UAbilitySystemComponent* ASC);
};
