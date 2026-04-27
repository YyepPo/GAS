#include "Actors/GAS_IcyCone.h"
#include "AbilityComponent/GAS_AbilitySystemComponent.h"
#include "Attributes/GAS_AttributeSetBase.h"
#include "Components/WidgetComponent.h"
#include "UI/Widget/OverlayWidget.h"

AGAS_IcyCone::AGAS_IcyCone()
{
	// Create ability system comp and mark it as replicated
	AbilitySystemComponent = CreateDefaultSubobject<UGAS_AbilitySystemComponent>(TEXT("Ability System Component"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UGAS_AttributeSetBase>(TEXT("Attribute Set"));
	
	bReplicates = true;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh Component"));
	SetRootComponent(StaticMeshComponent);
	
	HealthWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Health Widget Component"));
	HealthWidgetComponent->SetupAttachment(GetRootComponent());
	
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AGAS_IcyCone::BeginPlay()
{
	Super::BeginPlay();

	if (HealthWidgetComponent)
	{
		UOverlayWidget* WidgetClass = Cast<UOverlayWidget>(HealthWidgetComponent->GetWidget());
		if (WidgetClass)
		{
			WidgetClass->SetWidgetController(this);
		}
	}
	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		// Apply vital attributes
		if (VitalAttribute)
		{
			FGameplayEffectContextHandle EffectContextHandle = 
				AbilitySystemComponent->MakeEffectContext();
			EffectContextHandle.AddSourceObject(this);

			FGameplayEffectSpecHandle SpecHandle = 
				AbilitySystemComponent->MakeOutgoingSpec(
					VitalAttribute, 1, EffectContextHandle);

			if (SpecHandle.IsValid())
			{
				// Fixed: use ToSelf not ToTarget
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(
					*SpecHandle.Data.Get());
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("AGAS_IcyCone: VitalAttribute not set"));
		}

		// Bind health change delegate
		if (UGAS_AttributeSetBase* Set = Cast<UGAS_AttributeSetBase>(AttributeSet))
		{
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
				Set->GetHealthAttribute()).AddLambda(
				[this](const FOnAttributeChangeData& Data)
				{
					OnHealthChanged.Broadcast(Data.NewValue);
				});
		}
	}

}

UAbilitySystemComponent* AGAS_IcyCone::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
