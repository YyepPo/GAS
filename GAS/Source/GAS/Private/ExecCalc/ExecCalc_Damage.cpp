#include "ExecCalc/ExecCalc_Damage.h"

#include "GAS_GameplayTags.h"
#include "Abilities/GAS_AbilitySystemLibrary.h"
#include "AbilityComponent/GAS_FunctionLibrary.h"
#include "Attributes/GAS_AttributeSetBase.h"

struct GASDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitDamage);
	
	GASDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGAS_AttributeSetBase, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGAS_AttributeSetBase, CriticalHitChance, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGAS_AttributeSetBase, CriticalHitDamage, Source, false);
	}
};

static const GASDamageStatics& DamageStatics()
{
	static GASDamageStatics DStatics;
	return DStatics;
}

UExecCalc_Damage::UExecCalc_Damage()
{
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitDamageDef);
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
                                              FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	Super::Execute_Implementation(ExecutionParams, OutExecutionOutput);

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

    // Build evaluation parameters from captured tags
    FAggregatorEvaluateParameters EvaluationParameters;
    EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

    // --- Get base damage set by the ability via SetByCaller ---
    // Replace FGameplayTag::RequestGameplayTag with however you reference your damage tag
    float Damage = Spec.GetSetByCallerMagnitude(
         FGAS_GameplayTags::Get().Damage_Physical);

    // --- Armor ---
    float TargetArmor = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
        DamageStatics().ArmorDef, EvaluationParameters, TargetArmor);
    TargetArmor = FMath::Max(TargetArmor, 0.f);

    // Simple armor damage reduction — armor reduces damage by a flat percentage, capped at 75%
    const float ArmorDamageReduction = FMath::Clamp(TargetArmor / 100.f, 0.f, 0.75f);
    Damage *= (1.f - ArmorDamageReduction);

    // --- Critical Hit ---
    float SourceCriticalHitChance = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
        DamageStatics().CriticalHitChanceDef, EvaluationParameters, SourceCriticalHitChance);
    SourceCriticalHitChance = FMath::Max(SourceCriticalHitChance, 0.f);

    float SourceCriticalHitDamage = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
        DamageStatics().CriticalHitDamageDef, EvaluationParameters, SourceCriticalHitDamage);
    SourceCriticalHitDamage = FMath::Max(SourceCriticalHitDamage, 0.f);

    const bool bCriticalHit = FMath::RandRange(1, 100) < SourceCriticalHitChance;
    if (bCriticalHit)
    {
        // Double damage plus the bonus crit damage stat
        Damage = 2.f * Damage + SourceCriticalHitDamage;
    }

	FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();
    UGAS_FunctionLibrary::SetIsCriticalHit(EffectContextHandle, bCriticalHit);

    // --- Output ---
    const FGameplayModifierEvaluatedData EvaluatedData(
        UGAS_AttributeSetBase::GetIncomingDamageAttribute(),
        EGameplayModOp::Additive,
        Damage);
    OutExecutionOutput.AddOutputModifier(EvaluatedData);
}
