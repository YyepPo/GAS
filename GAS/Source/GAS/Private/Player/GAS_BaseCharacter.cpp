// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/Player/GAS_BaseCharacter.h"

#include "AbilitySystemComponent.h"
#include "Public/Player/GAS_PlayerState.h"


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

// Called to bind functionality to input
void AGAS_BaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AGAS_BaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Initialize AbilitySystemComponent on the server
	AGAS_PlayerState* PS = GetPlayerState<AGAS_PlayerState>();
	if (PS)
	{
		AbilitySystemComponent = PS->GetAbilitySystemComponent();
		AbilitySystemComponent->InitAbilityActorInfo(PS,this);
	}
}

void AGAS_BaseCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Initialize AbilitySystemComponent on the client
	AGAS_PlayerState* PS = GetPlayerState<AGAS_PlayerState>();
	if (PS)
	{
		AbilitySystemComponent = PS->GetAbilitySystemComponent();
		// Init ASC Actor Info for clients. Server will init its ASC when it possesses a new Actor.
		AbilitySystemComponent->InitAbilityActorInfo(PS,this);
	}
}
