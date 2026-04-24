#include "Public/Attributes/GAS_AttributeSetBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "GAS_GameplayTags.h"
#include "Abilities/GAS_AbilitySystemLibrary.h"
#include "AbilityComponent/GAS_FunctionLibrary.h"
#include "GameFramework/Character.h"
#include "Interface/CharacterInterface.h"
#include "Interface/CombatInterface.h"

UGAS_AttributeSetBase::UGAS_AttributeSetBase()
{

}

void UGAS_AttributeSetBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UGAS_AttributeSetBase, Health,           COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGAS_AttributeSetBase, MaxHealth,        COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGAS_AttributeSetBase, HealthRegenRate,  COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UGAS_AttributeSetBase, Mana,             COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGAS_AttributeSetBase, MaxMana,          COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UGAS_AttributeSetBase, Stamina,          COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGAS_AttributeSetBase, MaxStamina,       COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGAS_AttributeSetBase, StaminaRegenRate, COND_None, REPNOTIFY_Always);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UGAS_AttributeSetBase, Strength,         COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGAS_AttributeSetBase, Intelligence,     COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGAS_AttributeSetBase, Stealth,          COND_None, REPNOTIFY_Always);
}

void UGAS_AttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	// Clamp Health
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue,0,GetMaxHealth());
	}
	// Clamp Max Health, make sure MaxHealth stays above 1
	if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue,1);
	}
	// Clamp Mana
	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue,0,GetMaxMana());	
	}
	// Clamp Max Mana, make sure MaxMana stays above 1
	if (Attribute == GetMaxManaAttribute())
	{
		NewValue = FMath::Max(NewValue,1);
	}
	// Clamp Stamina
	if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue,0,GetMaxStamina());
	}
	// Clamp Max Stamina, make sure MaxStamina stays above 1
	if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Max(NewValue,1);
	}
	
}

void UGAS_AttributeSetBase::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	FEffectProperties EffectProperties;
	SetEffectProperties(Data,EffectProperties);
	
	// Clamp Health
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(),0,GetMaxHealth()));
	}

	// Clamp Mana
	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(),0,GetMaxMana()));
	}

	// Clamp Stamina
	if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(),0,GetMaxStamina()));
	}
	
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		HandleIncomingDamage(EffectProperties);
	}

	if (Data.EvaluatedData.Attribute == GetIncomingXpAttribute())
	{
		HandleIncomingXP(EffectProperties);
	}

}

void UGAS_AttributeSetBase::HandleIncomingDamage(const FEffectProperties& Props)
{
	const float LocalIncomingDamage = GetIncomingDamage();
	SetIncomingDamage(0.f);
	
	if (LocalIncomingDamage > 0.f)
	{
		const float NewHealth = GetHealth() - LocalIncomingDamage;
		SetHealth(FMath::Clamp(NewHealth,0.f,GetMaxHealth()));
		
		// Handle death & increase xp
		const bool bIsDead = NewHealth <= 0.f;
		if (bIsDead)
		{
			ICombatInterface* CombatInterface = Cast<ICombatInterface>(Props.TargetAvatarActor);
			if (CombatInterface)
			{
				CombatInterface->Die(UGAS_FunctionLibrary::GetDeathImpulse(Props.EffectContextHandle));
			}
			SendXPEvent(Props);
		}
		// Handle hit react
		else
		{
			if (Props.TargetCharacter->Implements<UCombatInterface>())
			{
				// Knockback
				const FVector KnockbackForce = UGAS_FunctionLibrary::GetKnockbackForce(Props.EffectContextHandle);
				if (!KnockbackForce.IsNearlyZero(1.f))
				{
					Props.TargetCharacter->LaunchCharacter(KnockbackForce, true, true);
				}

				// Hit react montage
				FGameplayTag HitReactTag = UGAS_AbilitySystemLibrary::CalculateHitDirection(
				Props.SourceCharacter, Props.TargetAvatarActor);
				
				Props.TargetASC->AddLooseGameplayTag(HitReactTag);

				FGameplayTagContainer TagContainer;
				TagContainer.AddTag(FGAS_GameplayTags::Get().Abilities_HitReact);
				Props.TargetASC->TryActivateAbilitiesByTag(TagContainer);
				
				Props.TargetASC->RemoveLooseGameplayTag(HitReactTag);
			}
		}
	}

	if (Props.SourceAvatarActor)
	{
		if (Props.SourceAvatarActor->Implements<UCharacterInterface>())
		{
			ICharacterInterface::Execute_ShowHitMarker(Props.SourceAvatarActor);
		}
	}
}

void UGAS_AttributeSetBase::SendXPEvent(const FEffectProperties& Props)
{
	if (Props.TargetAvatarActor->Implements<UCombatInterface>() == false)
	{
		return;
	}	
	
	const int32 TargetLevel = ICombatInterface::Execute_GetPlayerLevel(Props.TargetAvatarActor);
	const ECharacterClass TargetClass = ICombatInterface::Execute_GetCharacterClass(Props.TargetAvatarActor);
	const float XP = UGAS_FunctionLibrary::GetXPRewardForClassAndLevel(Props.TargetAvatarActor,TargetClass,TargetLevel);
	
	const FGAS_GameplayTags& GameplayTags = FGAS_GameplayTags::Get();
	FGameplayEventData Payload;
	Payload.EventTag =  GameplayTags.Attributes_Meta_IncomingXP;
	Payload.EventMagnitude = XP;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Props.SourceCharacter,
		GameplayTags.Attributes_Meta_IncomingXP,
		Payload);
}

void UGAS_AttributeSetBase::HandleIncomingXP(const FEffectProperties& Props)
{
	const float LocalIncomingXP = GetIncomingXp();
	SetIncomingXp(0.f);
	
	if (LocalIncomingXP <= 0.f)
	{
		return;
	}
	
	if (Props.SourceCharacter->Implements<UCharacterInterface>() && Props.SourceCharacter->Implements<UCombatInterface>())
	{
		const int32 CurrentLevel = ICombatInterface::Execute_GetPlayerLevel(Props.SourceCharacter);
		const int32 CurrentXP = ICharacterInterface::Execute_GetXP(Props.SourceCharacter);

		const int32 NewLevel = ICharacterInterface::Execute_FindLevelForXP(Props.SourceCharacter, CurrentXP + LocalIncomingXP);
		const int32 NumLevelUps = NewLevel - CurrentLevel;
		if (NumLevelUps > 0)
		{
			ICharacterInterface::Execute_AddToPlayerLevel(Props.SourceCharacter, NumLevelUps);

			int32 AttributePointsReward = 0;
			int32 SpellPointsReward = 0;

			for (int32 i = 0; i < NumLevelUps; ++i)
			{
				SpellPointsReward += ICharacterInterface::Execute_GetSpellPointsReward(Props.SourceCharacter, CurrentLevel + i);
				AttributePointsReward += ICharacterInterface::Execute_GetAttributePointsReward(Props.SourceCharacter, CurrentLevel + i);
			}
			
			ICharacterInterface::Execute_AddToAttributePoints(Props.SourceCharacter, AttributePointsReward);
			ICharacterInterface::Execute_AddToSpellPoints(Props.SourceCharacter, SpellPointsReward);
	

			ICharacterInterface::Execute_LevelUp(Props.SourceCharacter);
		}
			
		ICharacterInterface::Execute_AddToXP(Props.SourceCharacter, LocalIncomingXP);
	}
	else
	{
		UE_LOG(LogTemp,Warning,TEXT("QWE"));
	}
}

void UGAS_AttributeSetBase::SetEffectProperties(const FGameplayEffectModCallbackData& Data,
	FEffectProperties& Props) const
{
	Props.EffectContextHandle = Data.EffectSpec.GetContext();
	Props.SourceASC = Props.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();

	if (IsValid(Props.SourceASC) && Props.SourceASC->AbilityActorInfo.IsValid() && Props.SourceASC->AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.SourceAvatarActor = Props.SourceASC->AbilityActorInfo->AvatarActor.Get();
		Props.SourceController = Props.SourceASC->AbilityActorInfo->PlayerController.Get();
		if (Props.SourceController == nullptr && Props.SourceAvatarActor != nullptr)
		{
			if (const APawn* Pawn = Cast<APawn>(Props.SourceAvatarActor))
			{
				Props.SourceController = Pawn->GetController();
			}
		}
		if (Props.SourceController)
		{
			Props.SourceCharacter = Cast<ACharacter>(Props.SourceController->GetPawn());
		}
	}

	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		Props.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		Props.TargetCharacter = Cast<ACharacter>(Props.TargetAvatarActor);
		Props.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Props.TargetAvatarActor);
	}
}


void UGAS_AttributeSetBase::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
}

/// 
/// On Reps
/// 

void UGAS_AttributeSetBase::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_AttributeSetBase, Health, OldHealth);
}

void UGAS_AttributeSetBase::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_AttributeSetBase, MaxHealth, OldMaxHealth);
}

void UGAS_AttributeSetBase::OnRep_HealthRegenRate(const FGameplayAttributeData& OldHealthRegenRate) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_AttributeSetBase, HealthRegenRate, OldHealthRegenRate);
}

void UGAS_AttributeSetBase::OnRep_Mana(const FGameplayAttributeData& OldMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_AttributeSetBase, Mana, OldMana);
}

void UGAS_AttributeSetBase::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_AttributeSetBase, MaxMana, OldMaxMana);
}

void UGAS_AttributeSetBase::OnRep_Stamina(const FGameplayAttributeData& OldStamina) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_AttributeSetBase, Stamina, OldStamina);
}

void UGAS_AttributeSetBase::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_AttributeSetBase, MaxStamina, OldMaxStamina);
}

void UGAS_AttributeSetBase::OnRep_StaminaRegenRate(const FGameplayAttributeData& OldStaminaRegenRate) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_AttributeSetBase, StaminaRegenRate, OldStaminaRegenRate);
}

void UGAS_AttributeSetBase::OnRep_Strength(const FGameplayAttributeData& OldStrength) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_AttributeSetBase, Strength, OldStrength);
}

void UGAS_AttributeSetBase::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_AttributeSetBase, Intelligence, OldIntelligence);
}

void UGAS_AttributeSetBase::OnRep_Stealth(const FGameplayAttributeData& OldStealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_AttributeSetBase, Stealth, OldStealth);
}
