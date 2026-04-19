#include "GAS_AbilitySystemLibrary.h"
#include "GAS_GameplayTags.h"

FGameplayTag UGAS_AbilitySystemLibrary::CalculateHitDirection(AActor* SourceCharacter,AActor* TargetActor)
{
	// Safety checks - very important
	if (!SourceCharacter || !TargetActor)
	{
		return FGAS_GameplayTags::Get().Effects_HitReact_Front; // safe default
	}

	// Calculate direction from attacker to target
	const FVector DirectionToAttacker = (SourceCharacter->GetActorLocation() 
									   - TargetActor->GetActorLocation()).GetSafeNormal2D();

	const FVector VictimForward = TargetActor->GetActorForwardVector();

	// Better & cleaner way to get signed angle (-180 to 180)
	const float Dot = FVector::DotProduct(VictimForward, DirectionToAttacker);
	const float Angle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Dot, -1.0f, 1.0f)));

	// Determine quadrant using cross product for sign
	const float CrossZ = FVector::CrossProduct(VictimForward, DirectionToAttacker).Z;

	FGameplayTag HitDirectionTag;

	if (FMath::Abs(Angle) <= 45.0f)
	{
		HitDirectionTag = FGAS_GameplayTags::Get().Effects_HitReact_Front;
	}
	else if (FMath::Abs(Angle) >= 135.0f)
	{
		HitDirectionTag = FGAS_GameplayTags::Get().Effects_HitReact_Back;
	}
	else if (CrossZ > 0.0f)   // Note: sign depends on your coordinate system
	{
		HitDirectionTag = FGAS_GameplayTags::Get().Effects_HitReact_Right;
	}
	else
	{
		HitDirectionTag = FGAS_GameplayTags::Get().Effects_HitReact_Left;
	}

	return HitDirectionTag;
}
