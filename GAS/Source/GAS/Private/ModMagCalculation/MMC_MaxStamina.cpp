// Fill out your copyright notice in the Description page of Project Settings.


#include "ModMagCalculation/MMC_MaxStamina.h"

#include "Attributes/GAS_AttributeSetBase.h"
#include "Interface/CombatInterface.h"

UMMC_MaxStamina::UMMC_MaxStamina()
{
	StrengthRef.AttributeToCapture = UGAS_AttributeSetBase::GetStrengthAttribute();
	StrengthRef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	StrengthRef.bSnapshot = false;

	RelevantAttributesToCapture.Add(StrengthRef);
}

float UMMC_MaxStamina::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	float Strength = 0.f;
	GetCapturedAttributeMagnitude(StrengthRef, Spec, EvaluationParameters, Strength);
	Strength = FMath::Max<float>(Strength, 0.f);

	int32 PlayerLevel = 1;
	if (Spec.GetContext().GetSourceObject()->Implements<UCombatInterface>())
	{
		PlayerLevel = ICombatInterface::Execute_GetPlayerLevel(Spec.GetContext().GetSourceObject());
	}

	return 60.f + 2.5f * Strength + 10.f * PlayerLevel;
}
