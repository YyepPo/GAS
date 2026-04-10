#include "Public/Player/GAS_PlayerState.h"
#include "AbilitySystemComponent.h"
#include "Attributes/GAS_AttributeSetBase.h"

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

	// Another way of listening to attribute changes is to through Blueprint by using the node ListenForAttributeChange
	if (AbilitySystemComponent)
	{
		// Bind to attribute callbacks
		HealthChangeDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeBase->GetHealthAttribute()).AddUObject(this,&AGAS_PlayerState::OnHealthChanged);
		StaminaChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeBase->GetStaminaAttribute()).AddUObject(this,&AGAS_PlayerState::OnStaminaChanged);
		HealthChangeDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeBase->GetHealthAttribute()).AddUObject(this,&AGAS_PlayerState::OnManaChanged);
	}
}

UAbilitySystemComponent* AGAS_PlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AGAS_PlayerState::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	
}

void AGAS_PlayerState::OnStaminaChanged(const FOnAttributeChangeData& Data)
{
	
}

void AGAS_PlayerState::OnManaChanged(const FOnAttributeChangeData& Data)
{
	
}
