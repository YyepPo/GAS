// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/GAS_WidgetController.h"
#include "GASPlayerController.h"
#include "AbilityComponent/GAS_AbilitySystemComponent.h"
#include "Attributes/GAS_AttributeSetBase.h"
#include "Player/GAS_PlayerState.h"

UGAS_WidgetController::UGAS_WidgetController()
{
}

void UGAS_WidgetController::SetWidgetControllerParams(const FGAS_WidgetControllerParams& WCParams)
{
	PlayerController = WCParams.PlayerController;
	PlayerState = WCParams.PlayerState;
	AbilitySystemComponent = WCParams.AbilitySystemComponent;
	AttributeSet = WCParams.AttributeSet;
}

void UGAS_WidgetController::BroadcastInitialValues()
{
}

void UGAS_WidgetController::BindCallbacksToDependencies()
{
	
}

AGASPlayerController* UGAS_WidgetController::GetGASPC()
{
	if (GASPlayerController == nullptr)
	{
		GASPlayerController = Cast<AGASPlayerController>(PlayerController);
	}
	return GASPlayerController;
}

AGAS_PlayerState* UGAS_WidgetController::GetGASPS()
{
	if (GASPlayerState == nullptr)
	{
		GASPlayerState = Cast<AGAS_PlayerState>(PlayerState);
	}
	return GASPlayerState;
}

UGAS_AbilitySystemComponent* UGAS_WidgetController::GetGASASC()
{
	if (GASAbilitySystemComponent == nullptr)
	{
		GASAbilitySystemComponent = Cast<UGAS_AbilitySystemComponent>(AbilitySystemComponent);
	}
	return GASAbilitySystemComponent;
}

UGAS_AttributeSetBase* UGAS_WidgetController::GetGASAS()
{
	if (GASAttributeSet == nullptr)
	{
		GASAttributeSet = Cast<UGAS_AttributeSetBase>(AttributeSet);	
	}
	return GASAttributeSet;
}
