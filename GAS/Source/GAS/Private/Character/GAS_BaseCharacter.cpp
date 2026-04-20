// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/Character/GAS_BaseCharacter.h"
#include "GameFramework/Character.h"
#include "GAS_GameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemInterface.h"
#include "Public/Player/GAS_PlayerState.h"
#include "Components/CapsuleComponent.h"

class UAbilitySystemComponent;


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
	
	USkeletalMeshComponent* BodyMesh =	GetMesh();
	if (BodyMesh)
	{
		BodyMesh->SetSimulatePhysics(true);
		BodyMesh->SetEnableGravity(true);
		BodyMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
		BodyMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		BodyMesh->AddImpulse(DeathImpulse, NAME_None, true);	
	}
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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