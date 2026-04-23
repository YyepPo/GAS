#include "Character/GAS_Enemy.h"

#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "AbilityComponent/GAS_AbilitySystemComponent.h"
#include "AbilityComponent/GAS_FunctionLibrary.h"
#include "Attributes/GAS_AttributeSetBase.h"
#include "Components/WidgetComponent.h"
#include "UI/Widget/OverlayWidget.h"

AGAS_Enemy::AGAS_Enemy()
{
	PrimaryActorTick.bCanEverTick = false;
	
	// Create AbilitySystemComponent and mark it as replicated
	AbilitySystemComponent = CreateDefaultSubobject<UGAS_AbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	// Create Attribute Set
	AttributeSet = CreateDefaultSubobject<UGAS_AttributeSetBase>(TEXT("AttributeSet"));

	HealthWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthWidgetComponent"));
	HealthWidgetComponent->SetupAttachment(GetRootComponent());
}

int32 AGAS_Enemy::GetPlayerLevel_Implementation()
{
	return Level;
}

void AGAS_Enemy::BeginPlay()
{
	Super::BeginPlay();
	
	InitAbilityInfo(nullptr);
	ApplyDefaultAttributes();

	UOverlayWidget* HealthWidget = Cast<UOverlayWidget>(HealthWidgetComponent->GetWidget());
	if (HealthWidget)
	{
		HealthWidget->SetWidgetController(this);
	}

	if (UGAS_AttributeSetBase* Set = Cast<UGAS_AttributeSetBase>(AttributeSet))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Set->GetHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnHealthChanged.Broadcast(Data.NewValue);
		});

		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Set->GetMaxHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnMaxHealthChanged.Broadcast(Data.NewValue);
		});

		// These are used for initialization
		OnMaxHealthChanged.Broadcast(Set->GetHealth());
		OnMaxHealthChanged.Broadcast(Set->GetMaxHealth());
	}
}

void AGAS_Enemy::InitAbilityInfo(AController* NewController)
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
	Super::ApplyDefaultAttributes();
	//UGAS_FunctionLibrary::InitializeDefaultAttributes(this,CharacterClass,Level,AbilitySystemComponent);
}


