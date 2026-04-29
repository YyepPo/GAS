// Copyright Epic Games, Inc. All Rights Reserved.


#include "GASPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "GAS.h"
#include "AbilityComponent/GAS_AbilitySystemComponent.h"
#include "Input/GAS_AbilityEnhancedInput.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "Player/GAS_PlayerState.h"

void AGASPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogGAS, Error, TEXT("Could not spawn mobile controls widget."));

		}
	}

	GetASC();
}

void AGASPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
	
	// Add this log to confirm what class is actually being created
	UE_LOG(LogTemp, Warning, TEXT("aaaaaaaa -InputComponent class: %s"), 
		*InputComponent->GetClass()->GetName());

	if(UGAS_AbilityEnhancedInput* GASInputComp = Cast<UGAS_AbilityEnhancedInput>(InputComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("aaaaaaaa Cast SUCCEEDED")); // Will this print?
		GASInputComp->BindAbilityAction(InputConfig,this,&AGASPlayerController::AbilityInputPressed,&AGASPlayerController::AbilityInputReleased,&AGASPlayerController::AbilityInputHeld);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("aaaaaaaa Cast FAILED - wrong input component class"));
	}
}




bool AGASPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

UGAS_AbilitySystemComponent* AGASPlayerController::GetASC()
{
	if (AuroraAbilitySystemComponent == nullptr)
	{
		AuroraAbilitySystemComponent = Cast<UGAS_AbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return AuroraAbilitySystemComponent;
}

void AGASPlayerController::AbilityInputPressed(const FGameplayTag Tag)
{
	check(AuroraAbilitySystemComponent);

	AuroraAbilitySystemComponent->ActivateAbilityByTag(Tag);
}

void AGASPlayerController::AbilityInputReleased(const FGameplayTag Tag)
{
}

void AGASPlayerController::AbilityInputHeld(const FGameplayTag Tag)
{
}
