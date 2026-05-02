#include "Character/GAS_AuroraCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GASCharacter.h"
#include "GASPlayerController.h"
#include "GAS_GameplayTags.h"
#include "Data/LevelUpConfig.h"
#include "Input/GAS_AbilityEnhancedInput.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Player/GAS_PlayerState.h"
#include "UI/GAS_HUD.h"


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

void AGAS_AuroraCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGAS_AuroraCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGAS_AuroraCharacter::Look);
	}
}

void AGAS_AuroraCharacter::Move(const FInputActionValue& Value)
{
	if (AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(FGAS_GameplayTags::Get().Player_Block_InputPressed))
	{
		return;
	}
	
	const FVector2D InputAxisVector = Value.Get<FVector2D>();

	// Use controller yaw for movement direction (camera-relative movement)
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, InputAxisVector.Y);
	AddMovementInput(RightDirection,   InputAxisVector.X);
}

void AGAS_AuroraCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	// Horizontal — orbit camera around character
	AddControllerYawInput(LookAxisVector.X);
	
//	// Vertical — pitch camera up/down (clamped by SpringArm or CameraManager)
	AddControllerPitchInput(LookAxisVector.Y); 
}

void AGAS_AuroraCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	UE_LOG(LogTemp, Warning, TEXT("PossessedBy: Controller = %s"), *NewController->GetName());
	UE_LOG(LogTemp, Warning, TEXT("PossessedBy: PlayerState = %s"), 
		NewController->GetPlayerState<AGAS_PlayerState>() ? TEXT("VALID") : TEXT("NULL"));
	
	InitAbilityInfo();
	ApplyDefaultAttributes();
	AddCharacterAbilities();
}

void AGAS_AuroraCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	InitAbilityInfo();
}

void AGAS_AuroraCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	if (AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(FGAS_GameplayTags::Get().Player_Block_InputPressed))
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(FGAS_GameplayTags::Get().Player_Block_InputPressed);
	}
}

void AGAS_AuroraCharacter::InitAbilityInfo()
{
	AGAS_PlayerState* PS = GetPlayerState<AGAS_PlayerState>();
	if (PS == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitAbilityInfo: PlayerState is NULL"));
		return;
	}
	
	PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS,this);
	AbilitySystemComponent = PS->GetAbilitySystemComponent();

	AttributeSet = PS->GetAttributeSet();

	if (AGASPlayerController* GASPlayerController = Cast<AGASPlayerController>(GetController()))
	{
		if (AGAS_HUD* GASHUD = Cast<AGAS_HUD>(GASPlayerController->GetHUD()))
		{
			GASHUD->InitOverlay(GASPlayerController, PS, AbilitySystemComponent, AttributeSet);
		}
	}
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
	//Spawn level up effect inside the blueprint
}

void AGAS_AuroraCharacter::ShowHitMarker_Implementation()
{
	FGameplayEventData Payload;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		this,
		FGameplayTag::RequestGameplayTag("Event.HitConfirm"),
		Payload
	);

	if (HitCameraShake)
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			PlayerController->ClientStartCameraShake(HitCameraShake);
		}	
	}
}

/* IPlayerInterface Functions END **/
