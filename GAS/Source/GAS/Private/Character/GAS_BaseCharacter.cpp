// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/Character/GAS_BaseCharacter.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GAS_GameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Public/Player/GAS_PlayerState.h"
#include "Components/CapsuleComponent.h"
#include "GameplayEffect.h"
#include "AbilityComponent/GAS_AbilitySystemComponent.h"

// Sets default values
AGAS_BaseCharacter::AGAS_BaseCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Make sure that this actor replicated
	bReplicates = true;
}

// Called when the game starts or when spawned
void AGAS_BaseCharacter::BeginPlay()
{
	Super::BeginPlay();
}

UAbilitySystemComponent* AGAS_BaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

// Called every frame
void AGAS_BaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

/// 
// ICombatInterface Functions
/// 

UAnimMontage* AGAS_BaseCharacter::GetHitReactMontage_Implementation(const FGameplayTag& HitReactionTag)
{	
	if (!FrontHitReactMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("AGAS_BaseCharacter::GetHitReactMontage_Implementation: FrontHitReactMontage is not set on %s"), *GetName());
		return nullptr;
	}
	
	if (HitReactionTag == FGAS_GameplayTags::Get().Effects_HitReact_Front)
	{
		return FrontHitReactMontage;
	}
	if (HitReactionTag == FGAS_GameplayTags::Get().Effects_HitReact_Back)
	{
		return BackHitReactMontage ? BackHitReactMontage : FrontHitReactMontage;
	}
	if (HitReactionTag == FGAS_GameplayTags::Get().Effects_HitReact_Right)
	{
		return RightHitReactMontage ? RightHitReactMontage : FrontHitReactMontage;
	}
	if (HitReactionTag == FGAS_GameplayTags::Get().Effects_HitReact_Left)
	{
		return LeftHitReactMontage ? LeftHitReactMontage : FrontHitReactMontage;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("AGAS_BaseCharacter::GetHitReactMontage_Implementation: Unknown HitReactionTag (%s) on %s - using Front as fallback"),
		   *HitReactionTag.ToString(), *GetName());

	return FrontHitReactMontage;
}

ECharacterClass AGAS_BaseCharacter::GetCharacterClass_Implementation()
{
	return CharacterClass;
}

void AGAS_BaseCharacter::Die(const FVector& DeathImpulse)
{
	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this,DeathSound,GetActorLocation());
	}
	else
	{
		UE_LOG(LogTemp,Warning,TEXT("AGAS_BaseCharacter: Die: Death sound not selected"));
	}

	UE_LOG(LogTemp, Warning, TEXT("Die called with impulse: %s, magnitude: %.2f"), 
		   *DeathImpulse.ToString(), DeathImpulse.Size());

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	
	GetMesh()->AddImpulse(DeathImpulse, NAME_None, true);

	bIsDead = true;
		
	OnDeath.Broadcast(this);
}

FOnDeath& AGAS_BaseCharacter::GetOnDeathDelegate()
{
	return OnDeath;
}

FOnDamageSignature& AGAS_BaseCharacter::GetOnDamageSignature()
{
	return OnDamageSignature;
}

bool AGAS_BaseCharacter::IsDead_Implementation() const
{
	return bIsDead;
}

AActor* AGAS_BaseCharacter::GetAvatar_Implementation()
{
	return this;
}

/// 
// ICombatInterface Functions END
///

void AGAS_BaseCharacter::InitAbilityInfo()
{
	
}

void AGAS_BaseCharacter::AddCharacterAbilities()
{
	UGAS_AbilitySystemComponent* AuraASC = CastChecked<UGAS_AbilitySystemComponent>(AbilitySystemComponent);
	if (HasAuthority() == false)
	{
		return;
	}
	
	if (AuraASC)
	{
		AuraASC->AddCharacterAbilities(StartupAbilities);
	}
}

void AGAS_BaseCharacter::ApplyDefaultAttributes() const
{
	ApplyGameplayEffectToSelf(DefaultPrimaryAttributeClass,1);
	ApplyGameplayEffectToSelf(DefaultSecondaryAttributeClass,1);
	ApplyGameplayEffectToSelf(DefaultVitalAttributeClass,1);
}

void AGAS_BaseCharacter::ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> AttributeClass,float Level) const
{
	if (GetAbilitySystemComponent() == nullptr)
	{
		UE_LOG(LogTemp,Warning,TEXT("GAS_BaseCharacter::ApplyGameplayEffectToSelf: GetAbilitySystemComponent() is null"));
		return;
	}
	
	check(AttributeClass);

	FGameplayEffectContextHandle ContextHandler = GetAbilitySystemComponent()->MakeEffectContext();
	ContextHandler.AddSourceObject(this);	
	const FGameplayEffectSpecHandle EffectSpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(AttributeClass,Level,ContextHandler);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(),GetAbilitySystemComponent());
}
