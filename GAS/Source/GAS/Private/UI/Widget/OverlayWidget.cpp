// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/OverlayWidget.h"

void UOverlayWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	WidgetControllerSet();
}
