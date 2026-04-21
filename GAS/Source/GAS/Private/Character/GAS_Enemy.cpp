#include "Character/GAS_Enemy.h"

#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "AbilityComponent/GAS_AbilitySystemComponent.h"
#include "AbilityComponent/GAS_FunctionLibrary.h"

AGAS_Enemy::AGAS_Enemy()
{
	PrimaryActorTick.bCanEverTick = false;
	
	// Create AbilitySystemComponent and mark it as replicated
	AbilitySystemComponent = CreateDefaultSubobject<UGAS_AbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	// Create Attribute Set
	AttributeSet = CreateDefaultSubobject<UAttributeSet>(TEXT("AttributeSet"));
}

int32 AGAS_Enemy::GetPlayerLevel_Implementation()
{
	return Level;
}

void AGAS_Enemy::BeginPlay()
{
	Super::BeginPlay();
}

void AGAS_Enemy::InitAbilityInfo()
{
	if (AbilitySystemComponent == nullptr)
	{
		UE_LOG(LogTemp,Warning,TEXT("GAS_Enemy: InitAbilityInfo: AbilitySystemComponent is not valid"));
		return;
	}
	
	AbilitySystemComponent->InitAbilityActorInfo(this,this);
}

void AGAS_Enemy::ApplyDefaultAttributes() const
{
	UGAS_FunctionLibrary::InitializeDefaultAttributes(this,CharacterClass,Level,AbilitySystemComponent);
}


