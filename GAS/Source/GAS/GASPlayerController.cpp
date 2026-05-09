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

	if (IsLocalPlayerController())
	{
		// Fire every 0.1 seconds
		GetWorldTimerManager().SetTimer(
			TraceTimerHandle,
			this,
			&AGASPlayerController::PerformEnemyTrace,
			EnemyTraceInterval,
			true
		);
	}
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

	if(UGAS_AbilityEnhancedInput* GASInputComp = Cast<UGAS_AbilityEnhancedInput>(InputComponent))
	{
		GASInputComp->BindAbilityAction(InputConfig,this,&AGASPlayerController::AbilityInputPressed,
			&AGASPlayerController::AbilityInputReleased,
			&AGASPlayerController::AbilityInputHeld,
			&AGASPlayerController::AbilityInputConfirm,
			&AGASPlayerController::AbilityInputCancel);
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

	AuroraAbilitySystemComponent->OnAbilityInputPressed(Tag);
}

void AGASPlayerController::AbilityInputReleased(const FGameplayTag Tag)
{
	check(AuroraAbilitySystemComponent);

	AuroraAbilitySystemComponent->OnAbilityInputReleased(Tag);
}

void AGASPlayerController::AbilityInputHeld(const FGameplayTag Tag)
{
	check(AuroraAbilitySystemComponent);

	AuroraAbilitySystemComponent->OnAbilityInputHeld(Tag);
}

void AGASPlayerController::AbilityInputConfirm()
{
	check(AuroraAbilitySystemComponent);

	AuroraAbilitySystemComponent->OnAbilityInputConfirm();
}

void AGASPlayerController::AbilityInputCancel()
{
	check(AuroraAbilitySystemComponent);

	AuroraAbilitySystemComponent->OnAbilityInputCancel();
}

void AGASPlayerController::PerformEnemyTrace()
{
	if (PlayerCameraManager == nullptr)
	{
		return;
	}

	FVector Start = PlayerCameraManager->GetCameraLocation();
	FVector ForwardVector = PlayerCameraManager->GetActorForwardVector();

	FVector End = Start + (ForwardVector * DetectionTraceLength);

	FHitResult HitResult;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetPawn());

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Visibility,
		QueryParams
	);

//#if WITH_EDITOR
//	DrawDebugLine(
//		GetWorld(),
//		Start,
//		End,
//		bHit ? FColor::Red : FColor::Green,
//		false,
//		0.11f,
//		0,
//		1.5f
//	);
//#endif

	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();

		if (HitActor && UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor))
		{
			OnEnemyDetected.Broadcast(true);
		}
		else
		{
			OnEnemyDetected.Broadcast(false);
		}
	}
	else
	{
		OnEnemyDetected.Broadcast(false);
	}
}

