#include "GAS_AbilitySystemLibrary.h"
#include "GAS_GameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Player/GAS_PlayerState.h"
#include "UI/GAS_HUD.h"
#include "UI/WidgetController/GAS_WidgetController.h"

FGameplayTag UGAS_AbilitySystemLibrary::CalculateHitDirection(AActor* SourceCharacter,AActor* TargetActor)
{
	// Safety checks - very important
	if (!SourceCharacter || !TargetActor)
	{
		return FGAS_GameplayTags::Get().Effects_HitReact_Front; // safe default
	}

	const FVector ActorForwardVector = TargetActor->GetActorForwardVector();
	// Direction FROM target TO source (where the hit is coming from)
	const FVector DirectionToTarget = (SourceCharacter->GetActorLocation() - TargetActor->GetActorLocation()).GetSafeNormal();

	const float Acos = GetDirectionToTargetInDegress(ActorForwardVector,DirectionToTarget);

	FGameplayTag HitDirectionTag;

	if (Acos >= -45.f && Acos < 45.f)
	{
		HitDirectionTag = FGAS_GameplayTags::Get().Effects_HitReact_Front;
	}
	else if (Acos >= -135.f && Acos < -45.f)
	{
		HitDirectionTag = FGAS_GameplayTags::Get().Effects_HitReact_Left;
	}
	else if (Acos >= 45.f && Acos < 135.f)
	{
		HitDirectionTag = FGAS_GameplayTags::Get().Effects_HitReact_Right;
	}
	else
	{
		HitDirectionTag = FGAS_GameplayTags::Get().Effects_HitReact_Back;
	}

	return HitDirectionTag;
}

float UGAS_AbilitySystemLibrary::GetDirectionToTargetInDegress(const FVector& ActorForwardVector,
	const FVector& DirectionToTarget)
{
	float DotProduct =	FVector::DotProduct(ActorForwardVector,DirectionToTarget);

	float Acos = FMath::Acos(DotProduct);
	Acos = FMath::RadiansToDegrees(Acos);

	FVector CrossProduct = FVector::CrossProduct(ActorForwardVector, DirectionToTarget);
	(CrossProduct.Z < 0) ? Acos *= -1 : Acos *= 1;

	return Acos;
}

UGAS_AttributeWidgetController* UGAS_AbilitySystemLibrary::GetAttributeWidgetController(
	const UObject* WorldContextObject)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject,0);
	if (PlayerController == nullptr)
	{
		return nullptr;
	}
	
	AGAS_HUD* Hud = Cast<AGAS_HUD>(PlayerController->GetHUD());
	if (Hud == nullptr)
	{
		return nullptr;
	}
		
	AGAS_PlayerState* PlayerState = PlayerController->GetPlayerState<AGAS_PlayerState>();
	UAbilitySystemComponent* AbilitySystemComponent = PlayerState->GetAbilitySystemComponent();
	UAttributeSet* AttributeSet = PlayerState->GetAttributeSet();
	const FGAS_WidgetControllerParams Params (PlayerController, PlayerState,AbilitySystemComponent,AttributeSet);
	
	return Hud->GetAttributeWidgetController(Params);
}
