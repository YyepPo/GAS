#include "Character/GAS_Enemy.h"

#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "GAS_GameplayTags.h"
#include "AbilityComponent/GAS_AbilitySystemComponent.h"
#include "Attributes/GAS_AttributeSetBase.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "UI/Widget/OverlayWidget.h"
#

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

void AGAS_Enemy::Die(const FVector& DeathImpulse)
{
	Super::Die(DeathImpulse);
	if (HealthWidgetComponent)
	{
		HealthWidgetComponent->DestroyComponent();
	}

	if (StunParticleComp)
	{
		StunParticleComp->DestroyComponent();
	}
}

void AGAS_Enemy::BeginPlay()
{
	Super::BeginPlay();
	
	InitAbilityInfo();
	ApplyDefaultAttributes();
	AddCharacterAbilities();

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

	ListenForStackCountChange();
}

void AGAS_Enemy::InitAbilityInfo()
{
	if (AbilitySystemComponent == nullptr)
	{
		UE_LOG(LogTemp,Warning,TEXT("GAS_Enemy: InitAbilityInfo: AbilitySystemComponent is not valid"));
		return;
	}
	
	AbilitySystemComponent->InitAbilityActorInfo(this,this);
	AbilitySystemComponent->RegisterGameplayTagEvent(FGAS_GameplayTags::Get().Debuff_Stun, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AGAS_Enemy::StunTagChanged);
}

void AGAS_Enemy::ApplyDefaultAttributes() const
{
	Super::ApplyDefaultAttributes();
	//UGAS_FunctionLibrary::InitializeDefaultAttributes(this,CharacterClass,Level,AbilitySystemComponent);
}

void AGAS_Enemy::StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bIsStunned = NewCount > 0;

	if (NewCount > 0)
	{
		// Stop movement
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->StopMovementImmediately();

		// Stop any active abilities
		AbilitySystemComponent->CancelAllAbilities();

		if (StunStartParticle)
		{
			UGameplayStatics::SpawnEmitterAttached(StunStartParticle,GetMesh(),TEXT("FX_Stun"));
		}
		
		if (StunLoopParticle)
		{
			StunParticleComp = UGameplayStatics::SpawnEmitterAttached(StunLoopParticle,GetMesh(),TEXT("FX_Stun"));
		}

	}
	else if (NewCount == 0)
	{
		if (StunParticleComp)
		{
			StunParticleComp->DestroyComponent();
		}

		if (StunStopParticle)
		{
			UGameplayStatics::SpawnEmitterAttached(StunStopParticle,GetMesh(),TEXT("FX_Stun"));
		}
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}

	
	UE_LOG(LogTemp,Warning,TEXT("StunTagChanged: NewCount=%d"), NewCount);
}

void AGAS_Enemy::ListenForStackCountChange()
{
	if (AbilitySystemComponent == nullptr)
	{
		UE_LOG(LogTemp,Warning,TEXT("qwewqe"));
		return;
	}
	
	AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &AGAS_Enemy::OnActiveGameplayEffectAdded);

	AbilitySystemComponent->OnAnyGameplayEffectRemovedDelegate().AddUObject(this, &AGAS_Enemy::OnAnyGameplayEffectRemovedEvent);

	// Set whichever asset tag you put on GE_HitStack
	WatchedStackTag = FGameplayTag::RequestGameplayTag("GameplayEffect.HitStack");
}

void AGAS_Enemy::OnActiveGameplayEffectAdded(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec,
	FActiveGameplayEffectHandle Handle)
{
	// Filter to only your stacking GE by its asset tag
	FGameplayTagContainer AssetTags;
	Spec.GetAllAssetTags(AssetTags);

	if (!AssetTags.HasTagExact(WatchedStackTag))
		return;

	// Now bind the stack change delegate — same as the async task does internally
	FOnActiveGameplayEffectStackChange* StackDelegate =
		AbilitySystemComponent->OnGameplayEffectStackChangeDelegate(Handle);

	if (StackDelegate)
	{
		StackDelegate->AddUObject(this, &AGAS_Enemy::OnHitStackChanged);
	}
}

void AGAS_Enemy::OnAnyGameplayEffectRemovedEvent(const FActiveGameplayEffect& Effect)
{
}

void AGAS_Enemy::OnHitStackChanged(FActiveGameplayEffectHandle Handle, int32 NewCount, int32 OldCount)
{
	OnStackCountChanged.Broadcast(NewCount);
}
