#include "UI/WidgetController/GAS_OverlayWidgetController.h"

#include "GAS_GameplayTags.h"
#include "AbilityComponent/GAS_AbilitySystemComponent.h"
#include "Attributes/GAS_AttributeSetBase.h"

#include "Data/LevelUpConfig.h"
#include "Player/GAS_PlayerState.h"

void UGAS_OverlayWidgetController::BroadcastInitialValues()
{
	OnHealthChanged.Broadcast(GetGASAS()->GetHealth());
	OnMaxHealthChanged.Broadcast(GetGASAS()->GetMaxHealth());
	OnManaChanged.Broadcast(GetGASAS()->GetMana());
	OnMaxManaChanged.Broadcast(GetGASAS()->GetMaxMana());
	OnPlayerLevelChangedDelegate.Broadcast(0,false);
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

	if (GetGASASC())
	{
		GetGASASC()->AbilityEquippedEvent.AddUObject(this, &UGAS_OverlayWidgetController::OnAbilityEquipped);
		if (GetGASASC()->bStartupAbilitiesGiven)
		{
			BroadcastAbilityInfo();
		}
		else
		{
			GetGASASC()->AbilitiesGivenEvent.AddUObject(this, &UGAS_OverlayWidgetController::BroadcastAbilityInfo);
		}

	}

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

void UGAS_OverlayWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status,
	const FGameplayTag& Slot, const FGameplayTag& PrevSlot)
{
	const FGAS_GameplayTags& GameplayTags = FGAS_GameplayTags::Get();
	
	FGASAbilityInfo LastSlotInfo;
	LastSlotInfo.StatusTag = GameplayTags.Abilities_Status_Unlocked;
	LastSlotInfo.InputTag = PrevSlot;
	LastSlotInfo.AbilityTag = GameplayTags.Abilities_None;
	// Broadcast empty info if PreviousSlot is a valid slot. Only if equipping an already-equipped spell
	AbilityInfoDelegate.Broadcast(LastSlotInfo);
	
	FGASAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	Info.StatusTag = Status;
	Info.InputTag = Slot;
	AbilityInfoDelegate.Broadcast(Info);
}

void UGAS_OverlayWidgetController::BroadcastAbilityInfo()
{
	if (!GetGASASC()->bStartupAbilitiesGiven) return;

	FForEachAbility BroadcastDelegate;
	BroadcastDelegate.BindLambda([this](const FGameplayAbilitySpec& AbilitySpec)
	{
		FGASAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(GetGASASC()->GetAbilityTagFromSpec(AbilitySpec));
		Info.InputTag = GetGASASC()->GetInputTagFromSpec(AbilitySpec);
		Info.StatusTag = GetGASASC()->GetStatusFromSpec(AbilitySpec);
		AbilityInfoDelegate.Broadcast(Info);
	});
	GetGASASC()->ForEachAbility(BroadcastDelegate);
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
