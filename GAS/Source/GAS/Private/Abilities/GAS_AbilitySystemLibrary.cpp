#include "GAS_AbilitySystemLibrary.h"
#include "GAS_GameplayTags.h"

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
	
	/*// Calculate direction from attacker to target
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

	return HitDirectionTag;*/
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
