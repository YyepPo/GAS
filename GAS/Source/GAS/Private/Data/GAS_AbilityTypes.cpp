#include "Data/GAS_AbilityTypes.h"

bool FGAS_GameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = Super::NetSerialize(Ar, Map, bOutSuccess);

	bOutSuccess = Super::NetSerialize(Ar, Map, bOutSuccess);

	// Serialize your custom fields
	Ar << bIsBlockedHit;
	Ar << bIsCriticalHit;
	Ar << DamageType;
	Ar << DeathImpulse;
	Ar << KnockbackForce;
	Ar << bIsRadialDamage;
	Ar << RadialDamageInnerRadius;
	Ar << RadialDamageOuterRadius;
	Ar << RadialDamageOrigin;

	return true;
}
