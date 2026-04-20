#pragma once

#include "GameplayEffectTypes.h"
#include "GAS_AbilityTypes.generated.h"


USTRUCT(BlueprintType)
struct GAS_API FGAS_GameplayEffectContext : public FGameplayEffectContext
{
    GENERATED_BODY()

public:
    bool IsCriticalHit() const { return bIsCriticalHit; }
    bool IsBlockedHit() const { return bIsBlockedHit; }
    FGameplayTag GetDamageType() const { return DamageType; }
    FVector GetDeathImpulse() const { return DeathImpulse; }
    FVector GetKnockbackForce() const { return KnockbackForce; }
    bool IsRadialDamage() const { return bIsRadialDamage; }
    float GetRadialDamageInnerRadius() const { return RadialDamageInnerRadius; }
    float GetRadialDamageOuterRadius() const { return RadialDamageOuterRadius; }
    FVector GetRadialDamageOrigin() const { return RadialDamageOrigin; }

    void SetIsCriticalHit(bool bInIsCriticalHit) { bIsCriticalHit = bInIsCriticalHit; }
    void SetDamageType(FGameplayTag InDamageType) { DamageType = InDamageType; }
    void SetDeathImpulse(const FVector& InImpulse) { DeathImpulse = InImpulse; }
    void SetKnockbackForce(const FVector& InForce) { KnockbackForce = InForce; }
    void SetIsRadialDamage(bool bInIsRadialDamage) { bIsRadialDamage = bInIsRadialDamage; }
    void SetRadialDamageInnerRadius(float InRadialDamageInnerRadius) { RadialDamageInnerRadius = InRadialDamageInnerRadius; }
    void SetRadialDamageOuterRadius(float InRadialDamageOuterRadius) { RadialDamageOuterRadius = InRadialDamageOuterRadius; }
    void SetRadialDamageOrigin(const FVector& InRadialDamageOrigin) { RadialDamageOrigin = InRadialDamageOrigin; }

    /** REQUIRED OVERRIDES */
    virtual UScriptStruct* GetScriptStruct() const override
    {
        return FGAS_GameplayEffectContext::StaticStruct();
    }

    virtual FGAS_GameplayEffectContext* Duplicate() const override     // ← Return your own type!
    {
        FGAS_GameplayEffectContext* NewContext = new FGAS_GameplayEffectContext();
        *NewContext = *this;

        // Deep copy hit result if present
        if (GetHitResult())
        {
            NewContext->AddHitResult(*GetHitResult(), true);
        }

        return NewContext;
    }

    virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;

protected:
    UPROPERTY()
    bool bIsBlockedHit = false;

    UPROPERTY()
    bool bIsCriticalHit = false;

    /** TSharedPtr is dangerous for replication. Better to use FGameplayTag directly for now */
    UPROPERTY()
    FGameplayTag DamageType;                     // Changed from TSharedPtr

    UPROPERTY()
    FVector DeathImpulse = FVector::ZeroVector;

    UPROPERTY()
    FVector KnockbackForce = FVector::ZeroVector;

    UPROPERTY()
    bool bIsRadialDamage = false;

    UPROPERTY()
    float RadialDamageInnerRadius = 0.f;

    UPROPERTY()
    float RadialDamageOuterRadius = 0.f;

    UPROPERTY()
    FVector RadialDamageOrigin = FVector::ZeroVector;
};

template<>
struct TStructOpsTypeTraits<FGAS_GameplayEffectContext> : public TStructOpsTypeTraitsBase2<FGAS_GameplayEffectContext>
{
    enum
    {
        WithNetSerializer = true,
        WithCopy = true
    };
};