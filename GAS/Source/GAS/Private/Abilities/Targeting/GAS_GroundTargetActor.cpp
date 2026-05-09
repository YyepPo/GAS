#include "Abilities/Targeting/GAS_GroundTargetActor.h"

#include "Abilities/GameplayAbility.h"
#include "Components/DecalComponent.h"
#include "GameFramework/PlayerController.h"

AGAS_GroundTargetActor::AGAS_GroundTargetActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bDestroyOnConfirmation = true;
	ShouldProduceTargetDataOnServer = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	TargetDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("TargetDecal"));
	TargetDecal->SetupAttachment(SceneRoot);
	TargetDecal->DecalSize = FVector(DecalDepth, TargetRadius, TargetRadius);
	TargetDecal->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	TargetDecal->SetVisibility(false);

	SetReplicates(false);
}

void AGAS_GroundTargetActor::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);

	if (Ability && Ability->GetCurrentActorInfo())
	{
		SourceActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
		CachedPlayerController = Ability->GetCurrentActorInfo()->PlayerController.Get();
	}

	if (TargetDecal)
	{
		TargetDecal->DecalSize = FVector(DecalDepth, TargetRadius, TargetRadius);
		TargetDecal->SetDecalMaterial(DecalMaterial);
	}

	UpdateTargetingTrace();
}

void AGAS_GroundTargetActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateTargetingTrace();
}

bool AGAS_GroundTargetActor::IsConfirmTargetingAllowed()
{
	return bHasValidTarget;
}

void AGAS_GroundTargetActor::ConfirmTargetingAndContinue()
{
	if (!IsConfirmTargetingAllowed())
	{
		return;
	}

	FGameplayAbilityTargetDataHandle TargetData;
	CurrentHitResult.Location = CurrentHitResult.ImpactPoint;
	TargetData.Add(new FGameplayAbilityTargetData_SingleTargetHit(CurrentHitResult));
	TargetDataReadyDelegate.Broadcast(TargetData);
}

bool AGAS_GroundTargetActor::UpdateTargetingTrace()
{
	bHasValidTarget = false;

	APlayerController* PlayerController = CachedPlayerController.Get();
	if (PlayerController == nullptr)
	{
		if (TargetDecal)
		{
			TargetDecal->SetVisibility(false);
		}
		return false;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(GASGroundTargetTrace), false);
	QueryParams.bReturnPhysicalMaterial = false;
	if (SourceActor)
	{
		QueryParams.AddIgnoredActor(SourceActor);
	}

	FVector TraceStart = FVector::ZeroVector;
	FVector TraceDirection = FVector::ZeroVector;
	bool bHasTraceDirection = PlayerController->DeprojectMousePositionToWorld(TraceStart, TraceDirection);

	if (!bHasTraceDirection)
	{
		FRotator ViewRotation = FRotator::ZeroRotator;
		PlayerController->GetPlayerViewPoint(TraceStart, ViewRotation);
		TraceDirection = ViewRotation.Vector();
		bHasTraceDirection = true;
	}

	if (!bHasTraceDirection)
	{
		if (TargetDecal)
		{
			TargetDecal->SetVisibility(false);
		}
		return false;
	}

	FHitResult HitResult;
	const FVector TraceEnd = TraceStart + TraceDirection * MaxTraceDistance;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		TraceChannel,
		QueryParams);

	const bool bValidSurface = bHit && HitResult.bBlockingHit && HitResult.ImpactNormal.Z >= MinSurfaceNormalZ;
	if (!bValidSurface)
	{
		if (TargetDecal)
		{
			TargetDecal->SetVisibility(false);
		}
		return false;
	}

	CurrentHitResult = HitResult;
	bHasValidTarget = true;

	SetActorLocation(CurrentHitResult.ImpactPoint);
	SetActorRotation(FRotationMatrix::MakeFromX(-CurrentHitResult.ImpactNormal).Rotator());

	if (TargetDecal)
	{
		TargetDecal->SetVisibility(true);
		TargetDecal->DecalSize = FVector(DecalDepth, TargetRadius, TargetRadius);
	}

	return true;
}
