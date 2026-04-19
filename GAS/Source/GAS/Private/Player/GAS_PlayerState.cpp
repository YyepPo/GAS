#include "Public/Player/GAS_PlayerState.h"
#include "AbilitySystemComponent.h"
#include "Attributes/GAS_AttributeSetBase.h"
#include "Net/UnrealNetwork.h"

AGAS_PlayerState::AGAS_PlayerState()
{
	// Create ability system component and mark it as replicated
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("Ability System Component"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// increase the NetUpdateFrequency of your PlayerState.
	// It defaults to a very low value on the PlayerState and can cause delays or perceived lag before changes to things like Attributes and GameplayTags happen on the clients.
	// Be sure to enable Adaptive Network Update Frequency, Fortnite uses it.
	SetNetUpdateFrequency(100.f);
	
	// Attributes are replicated by default
	// Adding attribute set to owning actor of the asc automatically registers to that ASC
	AttributeBase = CreateDefaultSubobject<UGAS_AttributeSetBase>(TEXT("Attribute SET"));	
}

void AGAS_PlayerState::BeginPlay()
{
	Super::BeginPlay();
}

void AGAS_PlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AGAS_PlayerState,XP);
	DOREPLIFETIME(AGAS_PlayerState,Level);
	DOREPLIFETIME(AGAS_PlayerState,AttributePoints);
}

UAbilitySystemComponent* AGAS_PlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
void AGAS_PlayerState::AddToXP(int32 InXP)
{
	XP += InXP;
	OnXPChangedDelegate.Broadcast(XP);
}

void AGAS_PlayerState::AddToLevel(int32 InLevel)
{
	Level += InLevel;
	OnLevelChangedDelegate.Broadcast(Level, true);
}

void AGAS_PlayerState::SetXP(int32 InXP)
{
	XP = InXP;
	OnXPChangedDelegate.Broadcast(XP);
}

void AGAS_PlayerState::SetLevel(int32 InLevel)
{
	Level = InLevel;
	OnLevelChangedDelegate.Broadcast(Level, false);
}

void AGAS_PlayerState::SetAttributePoints(int32 InPoints)
{
	AttributePoints = InPoints;
	OnAttributePointsChangedDelegate.Broadcast(AttributePoints);
}

void AGAS_PlayerState::OnRep_Level(int32 OldLevel)
{
	OnLevelChangedDelegate.Broadcast(Level, true);
}

void AGAS_PlayerState::OnRep_XP(int32 OldXP)
{
	OnXPChangedDelegate.Broadcast(XP);
}

void AGAS_PlayerState::OnRep_AttributePoints(int32 OldAttributePoints)
{
	OnAttributePointsChangedDelegate.Broadcast(AttributePoints);
}

void AGAS_PlayerState::AddToAttributePoints(int32 InPoints)
{
	AttributePoints += InPoints;
	OnAttributePointsChangedDelegate.Broadcast(AttributePoints);
}