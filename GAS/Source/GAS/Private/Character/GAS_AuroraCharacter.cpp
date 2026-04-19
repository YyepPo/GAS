// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/GAS_AuroraCharacter.h"

#include "AbilitySystemComponent.h"
#include "Player/GAS_PlayerState.h"


// Sets default values
AGAS_AuroraCharacter::AGAS_AuroraCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AGAS_AuroraCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

UAbilitySystemComponent* AGAS_AuroraCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AGAS_AuroraCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitAbilityInfo();
}

void AGAS_AuroraCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	InitAbilityInfo();
}

// Called to bind functionality to input
void AGAS_AuroraCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AGAS_AuroraCharacter::InitAbilityInfo()
{
	AGAS_PlayerState* PS = GetPlayerState<AGAS_PlayerState>();
	if (PS == nullptr)
	{
		return;
	}
	
	PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS,this);
	AbilitySystemComponent = PS->GetAbilitySystemComponent();
	AttributeSet = PS->GetAttributeSet();
}

/* IPlayerInterface Functions **/

int32 AGAS_AuroraCharacter::FindLevelForXP_Implementation(int32 InXP) const
{
	return IPlayerInterface::FindLevelForXP_Implementation(InXP);
}

int32 AGAS_AuroraCharacter::GetXP_Implementation() const
{
	return IPlayerInterface::GetXP_Implementation();
}

int32 AGAS_AuroraCharacter::GetAttributePointsReward_Implementation(int32 Level) const
{
	return IPlayerInterface::GetAttributePointsReward_Implementation(Level);
}

int32 AGAS_AuroraCharacter::GetSpellPointsReward_Implementation(int32 Level) const
{
	return IPlayerInterface::GetSpellPointsReward_Implementation(Level);
}

void AGAS_AuroraCharacter::AddToXP_Implementation(int32 InXP)
{
	IPlayerInterface::AddToXP_Implementation(InXP);
}

void AGAS_AuroraCharacter::AddToPlayerLevel_Implementation(int32 InPlayerLevel)
{
	IPlayerInterface::AddToPlayerLevel_Implementation(InPlayerLevel);
}

void AGAS_AuroraCharacter::AddToAttributePoints_Implementation(int32 InAttributePoints)
{
	IPlayerInterface::AddToAttributePoints_Implementation(InAttributePoints);
}

int32 AGAS_AuroraCharacter::GetAttributePoints_Implementation() const
{
	return IPlayerInterface::GetAttributePoints_Implementation();
}

void AGAS_AuroraCharacter::AddToSpellPoints_Implementation(int32 InSpellPoints)
{
	IPlayerInterface::AddToSpellPoints_Implementation(InSpellPoints);
}

int32 AGAS_AuroraCharacter::GetSpellPoints_Implementation() const
{
	return IPlayerInterface::GetSpellPoints_Implementation();
}

void AGAS_AuroraCharacter::LevelUp_Implementation()
{
	IPlayerInterface::LevelUp_Implementation();
}

/* IPlayerInterface Functions END **/