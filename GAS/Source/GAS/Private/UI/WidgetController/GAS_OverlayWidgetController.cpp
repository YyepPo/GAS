#include "UI/WidgetController/GAS_OverlayWidgetController.h"
#include "Attributes/GAS_AttributeSetBase.h"
#include "Data/LevelUpConfig.h"
#include "Player/GAS_PlayerState.h"

void UGAS_OverlayWidgetController::BroadcastInitialValues()
{
	OnHealthChanged.Broadcast(GetGASAS()->GetHealth());
	OnMaxHealthChanged.Broadcast(GetGASAS()->GetMaxHealth());
	OnManaChanged.Broadcast(GetGASAS()->GetMana());
	OnMaxManaChanged.Broadcast(GetGASAS()->GetMaxMana());
}

void UGAS_OverlayWidgetController::BindCallbacksToDependencies()
{
	GetGASPS()->OnXPChangedDelegate.AddUObject(this, &UGAS_OverlayWidgetController::OnXPChanged);
	GetGASPS()->OnLevelChangedDelegate.AddLambda(
		[this](int32 NewLevel, bool bLevelUp)
		{
			OnPlayerLevelChangedDelegate.Broadcast(NewLevel, bLevelUp);
		}
	);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetGASAS()->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			}
		);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetGASAS()->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			}
		);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetGASAS()->GetManaAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnManaChanged.Broadcast(Data.NewValue);
			}
		);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetGASAS()->GetMaxManaAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxManaChanged.Broadcast(Data.NewValue);
			}
		);

	AbilitySystemComponent->AddGameplayEventTagContainerDelegate(
	   FGameplayTagContainer(FGameplayTag::RequestGameplayTag("Event.HitConfirm")),
	   FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(
		   this, &UGAS_OverlayWidgetController::OnHitConfirm)
   );
}

void UGAS_OverlayWidgetController::OnHitConfirm(FGameplayTag Tag, const FGameplayEventData* Payload)
{
	OnHitMarkerDelegate.Broadcast();
}

void UGAS_OverlayWidgetController::OnXPChanged(int32 NewXP)
{
	const ULevelUpConfig* LevelUpInfo = GetGASPS()->LevelUpConfig;
	checkf(LevelUpInfo, TEXT("Unabled to find LevelUpInfo. Please fill out AuraPlayerState Blueprint"));

	const int32 Level = LevelUpInfo->FindLevelForXP(NewXP);
	const int32 MaxLevel = LevelUpInfo->LevelUpInformation.Num();

	if (Level <= MaxLevel && Level > 0)
	{
		const int32 LevelUpRequirement = LevelUpInfo->LevelUpInformation[Level].LevelUpRequirement;
		const int32 PreviousLevelUpRequirement = LevelUpInfo->LevelUpInformation[Level - 1].LevelUpRequirement;

		const int32 DeltaLevelRequirement = LevelUpRequirement - PreviousLevelUpRequirement;
		const int32 XPForThisLevel = NewXP - PreviousLevelUpRequirement;

		const float XPBarPercent = static_cast<float>(XPForThisLevel) / static_cast<float>(DeltaLevelRequirement);

		OnXPPercentChangedDelegate.Broadcast(XPBarPercent);
	}
}
