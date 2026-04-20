#include "Character/GAS_AuroraCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilityComponent/GAS_AbilitySystemComponent.h"
#include "Data/LevelUpConfig.h"
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

void AGAS_AuroraCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitAbilityInfo();
	ApplyDefaultAttributes();
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
	AGAS_PlayerState* PS = GetController()->GetPlayerState<AGAS_PlayerState>();
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
	AGAS_PlayerState* GASPlayerState = GetController()->GetPlayerState<AGAS_PlayerState>();
	check(GASPlayerState);
	return GASPlayerState->LevelUpConfig->FindLevelForXP(InXP);
}

int32 AGAS_AuroraCharacter::GetXP_Implementation() const
{
	AGAS_PlayerState* GASPlayerState = GetController()->GetPlayerState<AGAS_PlayerState>();
	check(GASPlayerState);
	return GASPlayerState->GetXP();
}

int32 AGAS_AuroraCharacter::GetAttributePointsReward_Implementation(int32 Level) const
{
	AGAS_PlayerState* GASPlayerState = GetController()->GetPlayerState<AGAS_PlayerState>();
	check(GASPlayerState);
	return GASPlayerState->LevelUpConfig->LevelUpInformation[Level].AttributePointAward;
}

int32 AGAS_AuroraCharacter::GetSpellPointsReward_Implementation(int32 Level) const
{
	return 0;
}

void AGAS_AuroraCharacter::AddToXP_Implementation(int32 InXP)
{
	AGAS_PlayerState* GASPlayerState = GetController()->GetPlayerState<AGAS_PlayerState>();
	check(GASPlayerState);
	GASPlayerState->AddToXP(InXP);
}

void AGAS_AuroraCharacter::AddToPlayerLevel_Implementation(int32 InPlayerLevel)
{
	AGAS_PlayerState* GASPlayerState = GetController()->GetPlayerState<AGAS_PlayerState>();
	check(GASPlayerState);
	GASPlayerState->AddToLevel(InPlayerLevel);
}

void AGAS_AuroraCharacter::AddToAttributePoints_Implementation(int32 InAttributePoints)
{
	AGAS_PlayerState* GASPlayerState = GetController()->GetPlayerState<AGAS_PlayerState>();
	check(GASPlayerState);
	GASPlayerState->AddToAttributePoints(InAttributePoints);
}

int32 AGAS_AuroraCharacter::GetAttributePoints_Implementation() const
{
	AGAS_PlayerState* GASPlayerState = GetController()->GetPlayerState<AGAS_PlayerState>();
	check(GASPlayerState);
	return GASPlayerState->GetAttributePoints();
}

void AGAS_AuroraCharacter::AddToSpellPoints_Implementation(int32 InSpellPoints)
{
	
}

int32 AGAS_AuroraCharacter::GetSpellPoints_Implementation() const
{
	return 0;
}

void AGAS_AuroraCharacter::LevelUp_Implementation()
{
	//Spawn level up niagara effect	
}

/* IPlayerInterface Functions END **/