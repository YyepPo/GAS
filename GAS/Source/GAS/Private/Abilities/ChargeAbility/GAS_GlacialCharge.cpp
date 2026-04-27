#include "Abilities/ChargeAbility/GAS_GlacialCharge.h"

#include "Abilities/ChargeAbility/GAS_GlacialChargeIceBlock.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "AbilityComponent/GAS_FunctionLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SplineComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Interface/CombatInterface.h"
#include "TimerManager.h"

UGAS_GlacialCharge::UGAS_GlacialCharge()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGAS_GlacialCharge::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CachedCharacter = Character;
	bHasCleanedUp = false;
	DashSegments.Reset();
	DashSplineNormals.Reset();
	PendingIceBlockTransforms.Reset();
	SpawnedIceBlocks.Reset();
	CurrentDashSegmentIndex = 0;
	NextIceBlockIndex = 0;
	ActiveDashDistance = 0.f;
	DashSplineVerticalOffset = 0.f;
	EffectiveIceSpawnInterval = 0.f;
	DashStartTimeSeconds = 0.f;
	bDashInProgress = false;
	CachedDashDirection = FVector::ForwardVector;
	AffectedChargeTargets.Reset();
	CachedCapsuleComponent = Character->GetCapsuleComponent();
	bHasReleasedNearEndControl = false;
	bUseRightSideForCenterHit = true;

	const FVector DashDirection = GetDashDirection(Character);
	CachedDashDirection = DashDirection;
	if (!BuildDashSpline(Character, DashDirection))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	BuildDashSegments();
	BuildIceBlockTransforms();

	if (DashSegments.IsEmpty())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	OnChargeStarts();
	
	if (ChargeMontage)
	{
		ActiveMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ChargeMontage);
		ActiveMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageTaskFinished);
		ActiveMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageTaskFinished);
		ActiveMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageTaskFinished);
		ActiveMontageTask->ReadyForActivation();
	}

	ActiveChargeDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, ChargeStartupDelay);
	ActiveChargeDelayTask->OnFinish.AddDynamic(this, &ThisClass::OnChargeStartupDelayFinished);
	ActiveChargeDelayTask->ReadyForActivation();
}

void UGAS_GlacialCharge::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	CleanupChargeState();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGAS_GlacialCharge::CreateOrResetDashSpline(ACharacter* Character)
{
	if (DashSpline == nullptr)
	{
		DashSpline = NewObject<USplineComponent>(Character, TEXT("GlacialChargeSpline"));
		DashSpline->CreationMethod = EComponentCreationMethod::Instance;
		DashSpline->SetAbsolute(true, true, true);
		DashSpline->SetVisibility(bDrawDebug);
		DashSpline->RegisterComponent();
	}

	DashSpline->ClearSplinePoints(false);
	DashSpline->SetWorldTransform(FTransform::Identity);
}

bool UGAS_GlacialCharge::BuildDashSpline(ACharacter* Character, const FVector& DashDirection)
{
	if (Character == nullptr || DashDirection.IsNearlyZero())
	{
		return false;
	}

	CreateOrResetDashSpline(Character);

	UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
	const float CapsuleHalfHeight = Capsule ? Capsule->GetScaledCapsuleHalfHeight() : 88.f;
	const FVector ActorLocation = Character->GetActorLocation();
	const FVector BaseSampleLocation = ActorLocation - FVector::UpVector * CapsuleHalfHeight;
	DashSplineVerticalOffset = CapsuleHalfHeight + TrailSurfaceOffset;
	const FVector SplineHeightOffset(0.f, 0.f, DashSplineVerticalOffset);

	TArray<FVector> SplinePoints;
	TArray<FVector> SurfaceNormals;
	SplinePoints.Reserve(TraceCount + 3);
	SurfaceNormals.Reserve(TraceCount + 3);

	FHitResult InitialGroundHit;
	FVector LastGroundPoint = BaseSampleLocation;
	FVector LastSurfaceNormal = FVector::UpVector;
	if (TraceGroundSample(Character, BaseSampleLocation, InitialGroundHit))
	{
		LastGroundPoint = InitialGroundHit.ImpactPoint;
		LastSurfaceNormal = InitialGroundHit.ImpactNormal.GetSafeNormal();
	}

	SplinePoints.Add(LastGroundPoint + SplineHeightOffset);
	SurfaceNormals.Add(LastSurfaceNormal);

	int32 ConsecutiveMisses = 0;
	for (int32 TraceIndex = 1; TraceIndex <= TraceCount; ++TraceIndex)
	{
		const FVector TraceSample = BaseSampleLocation + DashDirection * (TraceSpacing * TraceIndex);
		FHitResult GroundHit;

		if (!TraceGroundSample(Character, TraceSample, GroundHit))
		{
			++ConsecutiveMisses;
			if (ConsecutiveMisses > 1)
			{
				break;
			}

			const FVector FallbackGroundPoint = LastGroundPoint + DashDirection * TraceSpacing;
			SplinePoints.Add(FallbackGroundPoint + SplineHeightOffset);
			SurfaceNormals.Add(LastSurfaceNormal);
			LastGroundPoint = FallbackGroundPoint;
			continue;
		}

		ConsecutiveMisses = 0;

		const FVector GroundPoint = GroundHit.ImpactPoint;
		const FVector SurfaceNormal = GroundHit.ImpactNormal.GetSafeNormal();
		const float HeightDelta = GroundPoint.Z - LastGroundPoint.Z;

		if (HeightDelta > MaxClimbableHeight)
		{
			break;
		}

		if (HeightDelta >= MinClimbableHeight)
		{
			FVector BridgeGroundPoint = FMath::Lerp(LastGroundPoint, GroundPoint, 0.5f);
			BridgeGroundPoint.Z = FMath::Lerp(LastGroundPoint.Z, GroundPoint.Z, 0.65f);

			SplinePoints.Add(BridgeGroundPoint + SplineHeightOffset);
			SurfaceNormals.Add(FMath::Lerp(LastSurfaceNormal, SurfaceNormal, 0.5f).GetSafeNormal());
		}

		SplinePoints.Add(GroundPoint + SplineHeightOffset);
		SurfaceNormals.Add(SurfaceNormal);
		LastGroundPoint = GroundPoint;
		LastSurfaceNormal = SurfaceNormal;
	}

	if (SplinePoints.Num() < 2)
	{
		return false;
	}

	DashSplineNormals = SurfaceNormals;
	for (int32 PointIndex = 0; PointIndex < SplinePoints.Num(); ++PointIndex)
	{
		DashSpline->AddSplinePoint(SplinePoints[PointIndex], ESplineCoordinateSpace::World, false);

		const ESplinePointType::Type PointType =
			(PointIndex == 0 || PointIndex == SplinePoints.Num() - 1) ? ESplinePointType::CurveClamped : ESplinePointType::Curve;
		DashSpline->SetSplinePointType(PointIndex, PointType, false);
	}

	for (int32 PointIndex = 0; PointIndex < SplinePoints.Num(); ++PointIndex)
	{
		const FVector PrevPoint = SplinePoints[FMath::Max(PointIndex - 1, 0)];
		const FVector NextPoint = SplinePoints[FMath::Min(PointIndex + 1, SplinePoints.Num() - 1)];
		const FVector Tangent = (NextPoint - PrevPoint) * SplineTension;
		DashSpline->SetTangentsAtSplinePoint(PointIndex, Tangent, Tangent, ESplineCoordinateSpace::World, false);
	}

	DashSpline->UpdateSpline();

	if (bDrawDebug)
	{
		const float SplineLength = DashSpline->GetSplineLength();
		for (float Distance = 0.f; Distance < SplineLength; Distance += 50.f)
		{
			const FVector DebugLocation = DashSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
			DrawDebugSphere(GetWorld(), DebugLocation, 10.f, 8, FColor::Cyan, false, 3.f);
		}
	}

	return true;
}

void UGAS_GlacialCharge::BuildDashSegments()
{
	DashSegments.Reset();

	if (DashSpline == nullptr || DashDuration <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const float SplineLength = DashSpline->GetSplineLength();
	ActiveDashDistance = FMath::Min(SplineLength, DashForce * DashDuration);
	if (ActiveDashDistance <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const float SegmentDistanceStep = FMath::Max(10.f, DashSegmentLength);
	const float SegmentSpeed = ActiveDashDistance / DashDuration;
	const int32 SegmentCount = FMath::Max(1, FMath::CeilToInt(ActiveDashDistance / SegmentDistanceStep));

	float CurrentDistance = 0.f;
	for (int32 SegmentIndex = 0; SegmentIndex < SegmentCount; ++SegmentIndex)
	{
		const float NextDistance = FMath::Min(ActiveDashDistance, CurrentDistance + SegmentDistanceStep);
		const FVector StartLocation = DashSpline->GetLocationAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World);
		const FVector EndLocation = DashSpline->GetLocationAtDistanceAlongSpline(NextDistance, ESplineCoordinateSpace::World);
		const float SegmentDistance = FVector::Distance(StartLocation, EndLocation);
		if (SegmentDistance <= KINDA_SMALL_NUMBER)
		{
			CurrentDistance = NextDistance;
			continue;
		}

		FGlacialChargeDashSegment DashSegment;
		DashSegment.Direction = (EndLocation - StartLocation).GetSafeNormal();
		DashSegment.Distance = SegmentDistance;
		DashSegment.Duration = DashDuration * (SegmentDistance / ActiveDashDistance);
		DashSegment.Strength = SegmentSpeed;
		DashSegments.Add(DashSegment);

		CurrentDistance = NextDistance;
	}
}

void UGAS_GlacialCharge::BuildIceBlockTransforms()
{
	PendingIceBlockTransforms.Reset();

	if (DashSpline == nullptr || ActiveDashDistance <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const float Spacing = FMath::Max(10.f, BlockSpacing);
	const float FirstDistance = FMath::Min(FMath::Max(0.f, BlockSpawnLeadDistance), ActiveDashDistance);

	for (float DistanceAlongSpline = FirstDistance; DistanceAlongSpline <= ActiveDashDistance; DistanceAlongSpline += Spacing)
	{
		PendingIceBlockTransforms.Add(MakeIceBlockTransform(DistanceAlongSpline));
	}

	if (PendingIceBlockTransforms.IsEmpty())
	{
		PendingIceBlockTransforms.Add(MakeIceBlockTransform(ActiveDashDistance));
	}
	else
	{
		const FVector LastSpawnLocation = PendingIceBlockTransforms.Last().GetLocation();
		const FVector EndLocation = MakeIceBlockTransform(ActiveDashDistance).GetLocation();
		if (!LastSpawnLocation.Equals(EndLocation, 1.f))
		{
			PendingIceBlockTransforms.Add(MakeIceBlockTransform(ActiveDashDistance));
		}
	}
}

void UGAS_GlacialCharge::StartNextDashSegment()
{
	if (CurrentDashSegmentIndex < 0 || CurrentDashSegmentIndex >= DashSegments.Num())
	{
		SnapCharacterToSplineDistance(ActiveDashDistance, true);
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	const FGlacialChargeDashSegment& DashSegment = DashSegments[CurrentDashSegmentIndex];
	if (DashSegment.Duration <= KINDA_SMALL_NUMBER || DashSegment.Direction.IsNearlyZero())
	{
		++CurrentDashSegmentIndex;
		StartNextDashSegment();
		return;
	}

	const bool bIsLastSegment = CurrentDashSegmentIndex == DashSegments.Num() - 1;
	const ERootMotionFinishVelocityMode FinishVelocityMode =
		bIsLastSegment ? ERootMotionFinishVelocityMode::SetVelocity : ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity;

	ActiveRootMotionTask = UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
		this,
		FName(*FString::Printf(TEXT("GlacialChargeSegment_%d"), CurrentDashSegmentIndex)),
		DashSegment.Direction,
		DashSegment.Strength,
		DashSegment.Duration,
		false,
		nullptr,
		FinishVelocityMode,
		FVector::ZeroVector,
		0.f,
		false);

	ActiveRootMotionTask->OnFinish.AddDynamic(this, &ThisClass::OnDashSegmentFinished);
	ActiveRootMotionTask->ReadyForActivation();
}

void UGAS_GlacialCharge::StartTrailSpawning()
{
	ACharacter* Character = CachedCharacter.Get();
	if (Character == nullptr || !Character->HasAuthority() || PendingIceBlockTransforms.IsEmpty())
	{
		return;
	}

	const float AverageDashSpeed = ActiveDashDistance / FMath::Max(DashDuration, KINDA_SMALL_NUMBER);
	const float TimeToTraverseOneBlock = BlockSpacing / FMath::Max(AverageDashSpeed, 1.f);
	EffectiveIceSpawnInterval = FMath::Min(IceSpawnInterval, TimeToTraverseOneBlock * 0.75f);
	EffectiveIceSpawnInterval = FMath::Max(0.01f, EffectiveIceSpawnInterval);

	SpawnNextIceBlock();

	if (NextIceBlockIndex < PendingIceBlockTransforms.Num())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(IceSpawnTimerHandle, this, &ThisClass::SpawnNextIceBlock, EffectiveIceSpawnInterval, true);
		}
	}
}

void UGAS_GlacialCharge::SpawnNextIceBlock()
{
	ACharacter* Character = CachedCharacter.Get();
	if (Character == nullptr || IceBlockClass == nullptr || NextIceBlockIndex >= PendingIceBlockTransforms.Num())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(IceSpawnTimerHandle);
		}
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = Character;
	SpawnParameters.Instigator = Character;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	FTransform SpawnTransform;
	const bool bHasValidSpawnTransform = ResolveIceBlockSpawnTransform(PendingIceBlockTransforms[NextIceBlockIndex], SpawnTransform);
	if (bHasValidSpawnTransform)
	{
		AGAS_GlacialChargeIceBlock* IceBlock = GetWorld()->SpawnActor<AGAS_GlacialChargeIceBlock>(
			IceBlockClass,
			SpawnTransform,
			SpawnParameters);

		if (IceBlock)
		{
			IceBlock->InitializeIceBlock(IceBlockDimensions, Character);
			SpawnedIceBlocks.Add(IceBlock);

			if (UCapsuleComponent* CapsuleComponent = Character->GetCapsuleComponent())
			{
				CapsuleComponent->IgnoreActorWhenMoving(IceBlock, true);
			}
		}
	}

	++NextIceBlockIndex;
	if (NextIceBlockIndex >= PendingIceBlockTransforms.Num())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(IceSpawnTimerHandle);
		}
	}
}

bool UGAS_GlacialCharge::ResolveIceBlockSpawnTransform(const FTransform& IntendedTransform, FTransform& OutSpawnTransform) const
{
	ACharacter* Character = CachedCharacter.Get();
	UWorld* World = GetWorld();
	if (Character == nullptr || World == nullptr)
	{
		return false;
	}

	FCollisionObjectQueryParams GroundQueryParams;
	GroundQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	GroundQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	FCollisionQueryParams GroundTraceParams(SCENE_QUERY_STAT(GlacialChargeIceGroundTrace), false, Character);
	GroundTraceParams.AddIgnoredActor(Character);

	for (const TWeakObjectPtr<AGAS_GlacialChargeIceBlock>& IceBlockPtr : SpawnedIceBlocks)
	{
		if (AGAS_GlacialChargeIceBlock* SpawnedIceBlock = IceBlockPtr.Get())
		{
			GroundTraceParams.AddIgnoredActor(SpawnedIceBlock);
		}
	}

	const FVector IntendedLocation = IntendedTransform.GetLocation();
	const FVector TraceStart = IntendedLocation + (FVector::UpVector * FMath::Max(10.f, IceGroundTraceStartHeight));
	const FVector TraceEnd = IntendedLocation - (FVector::UpVector * FMath::Max(10.f, IceGroundTraceDepth));

	FHitResult GroundHit;
	const bool bHitGround = World->LineTraceSingleByObjectType(
		GroundHit,
		TraceStart,
		TraceEnd,
		GroundQueryParams,
		GroundTraceParams);

	if (bDrawDebug)
	{
		DrawDebugLine(World, TraceStart, TraceEnd, bHitGround ? FColor::Cyan : FColor::Purple, false, 1.f, 0, 1.25f);
	}

	if (!bHitGround)
	{
		return false;
	}

	FVector Tangent = IntendedTransform.GetRotation().GetAxisX().GetSafeNormal();
	if (Tangent.IsNearlyZero())
	{
		Tangent = CachedDashDirection.GetSafeNormal2D();
	}

	FVector SurfaceNormal = GroundHit.ImpactNormal.GetSafeNormal();
	if (SurfaceNormal.IsNearlyZero())
	{
		SurfaceNormal = FVector::UpVector;
	}

	FTransform ResolvedTransform(FRotationMatrix::MakeFromXZ(Tangent, SurfaceNormal).ToQuat(), GroundHit.ImpactPoint, FVector::OneVector);
	FVector ResolvedLocation = GroundHit.ImpactPoint;
	ResolvedLocation.Z += GetIceBlockWorldHalfHeight(ResolvedTransform) + TrailSurfaceOffset;
	ResolvedTransform.SetLocation(ResolvedLocation);

	FCollisionObjectQueryParams PawnQueryParams;
	PawnQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams PawnOverlapParams(SCENE_QUERY_STAT(GlacialChargeIceEnemyOverlap), false, Character);
	PawnOverlapParams.AddIgnoredActor(Character);

	TArray<FOverlapResult> OverlapResults;
	const FVector HalfExtents = (IceBlockDimensions * 0.5f).ComponentMax(FVector(1.f, 1.f, 1.f));
	World->OverlapMultiByObjectType(
		OverlapResults,
		ResolvedTransform.GetLocation(),
		ResolvedTransform.GetRotation(),
		PawnQueryParams,
		FCollisionShape::MakeBox(HalfExtents),
		PawnOverlapParams);

	float MaxAllowedTopZ = TNumericLimits<float>::Max();
	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AActor* OverlapActor = OverlapResult.GetActor();
		if (!IsValidChargeTarget(OverlapActor))
		{
			continue;
		}

		MaxAllowedTopZ = FMath::Min(MaxAllowedTopZ, GetActorFeetZ(OverlapActor) - EnemyFootClearance);
	}

	if (MaxAllowedTopZ < TNumericLimits<float>::Max())
	{
		const float CurrentTopZ = ResolvedTransform.GetLocation().Z + GetIceBlockWorldHalfHeight(ResolvedTransform);
		if (CurrentTopZ > MaxAllowedTopZ)
		{
			FVector AdjustedLocation = ResolvedTransform.GetLocation();
			AdjustedLocation.Z -= (CurrentTopZ - MaxAllowedTopZ);
			ResolvedTransform.SetLocation(AdjustedLocation);
		}
	}

	OutSpawnTransform = ResolvedTransform;
	return true;
}

void UGAS_GlacialCharge::StartChargeExecution()
{
	ACharacter* Character = CachedCharacter.Get();
	if (Character == nullptr || bDashInProgress)
	{
		return;
	}

	if (UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement())
	{
		SavedMovementMode = CharacterMovement->MovementMode;
		SavedCustomMovementMode = CharacterMovement->CustomMovementMode;
		bHasSavedMovementMode = true;
		CharacterMovement->SetMovementMode(MOVE_Flying);
	}

	UGAS_FunctionLibrary::ApplyBlockMovementTag(GetAbilitySystemComponentFromActorInfo());
	ConfigureOwnerTrailCollision(true);

	const FRotator DashYawRotation = CachedDashDirection.Rotation();
	Character->SetActorRotation(FRotator(0.f, DashYawRotation.Yaw, 0.f));
	DashStartTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	bDashInProgress = true;
	StartSplineCorrection();
	StartChargeHitSweep();
	SnapCharacterToSplineDistance(0.f, true);
	StartTrailSpawning();
	StartNextDashSegment();
}

void UGAS_GlacialCharge::StartChargeHitSweep()
{
	ACharacter* Character = CachedCharacter.Get();
	UWorld* World = GetWorld();
	if (!bDashInProgress || Character == nullptr || World == nullptr || !Character->HasAuthority())
	{
		return;
	}

	UpdateChargeHitSweep();

	World->GetTimerManager().SetTimer(
		ChargeHitSweepTimerHandle,
		this,
		&ThisClass::UpdateChargeHitSweep,
		FMath::Max(0.01f, SweepTraceInterval),
		true);
}

void UGAS_GlacialCharge::UpdateChargeHitSweep()
{
	ACharacter* Character = CachedCharacter.Get();
	UWorld* World = GetWorld();
	if (!bDashInProgress || Character == nullptr || World == nullptr || !Character->HasAuthority())
	{
		return;
	}

	FVector DashForward = Character->GetActorForwardVector().GetSafeNormal2D();
	if (DashForward.IsNearlyZero())
	{
		DashForward = CachedDashDirection.GetSafeNormal2D();
	}

	if (DashForward.IsNearlyZero())
	{
		return;
	}

	const FVector SweepOrigin =
		Character->GetActorLocation() +
		(DashForward * SweepTraceForwardOffset) +
		(FVector::UpVector * SweepTraceHeightOffset);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(GlacialChargeEnemySweep), false, Character);
	QueryParams.AddIgnoredActor(Character);

	for (const TWeakObjectPtr<AGAS_GlacialChargeIceBlock>& IceBlockPtr : SpawnedIceBlocks)
	{
		if (AGAS_GlacialChargeIceBlock* IceBlock = IceBlockPtr.Get())
		{
			QueryParams.AddIgnoredActor(IceBlock);
		}
	}

	const float HalfArc = FMath::Max(0.f, SweepArcDegrees * 0.5f);
	const float AngleStep = FMath::Max(1.f, SweepAngleStepDegrees);

	for (float TraceAngle = -HalfArc; TraceAngle <= HalfArc + KINDA_SMALL_NUMBER; TraceAngle += AngleStep)
	{
		const FVector SweepDirection = FRotator(0.f, TraceAngle, 0.f).RotateVector(DashForward).GetSafeNormal();
		if (SweepDirection.IsNearlyZero())
		{
			continue;
		}

		const FVector TraceEnd = SweepOrigin + (SweepDirection * SweepTraceRange);
		TArray<FHitResult> HitResults;
		const bool bHit = World->LineTraceMultiByObjectType(HitResults, SweepOrigin, TraceEnd, ObjectQueryParams, QueryParams);

		if (bDrawDebug)
		{
			DrawDebugLine(World, SweepOrigin, TraceEnd, bHit ? FColor::Orange : FColor::Silver, false, SweepTraceInterval * 1.5f, 0, 1.25f);
		}

		if (!bHit)
		{
			continue;
		}

		for (const FHitResult& HitResult : HitResults)
		{
			HandleChargeSweepHit(HitResult.GetActor(), SweepOrigin, DashForward, TraceAngle);
		}
	}
}

void UGAS_GlacialCharge::HandleChargeSweepHit(AActor* HitActor, const FVector& SweepOrigin, const FVector& DashForward, float TraceAngleDegrees)
{
	if (!IsValidChargeTarget(HitActor))
	{
		return;
	}

	const TWeakObjectPtr<AActor> TargetPtr(HitActor);
	if (AffectedChargeTargets.Contains(TargetPtr))
	{
		return;
	}

	const FVector KnockbackDirection = ComputeSideKnockbackDirection(HitActor, DashForward, TraceAngleDegrees);
	if (KnockbackDirection.IsNearlyZero())
	{
		return;
	}

	AffectedChargeTargets.Add(TargetPtr);

	ApplyDamageEffect(HitActor);

	const FVector LaunchVelocity = (KnockbackDirection * EnemyKnockbackForce) + (FVector::UpVector * EnemyKnockbackUpForce);
	if (ACharacter* HitCharacter = Cast<ACharacter>(HitActor))
	{
		HitCharacter->LaunchCharacter(LaunchVelocity, true, true);
	}
	else if (UPrimitiveComponent* RootPrimitiveComponent = Cast<UPrimitiveComponent>(HitActor->GetRootComponent()))
	{
		if (RootPrimitiveComponent->IsSimulatingPhysics())
		{
			RootPrimitiveComponent->AddImpulse(LaunchVelocity * RootPrimitiveComponent->GetMass());
		}
	}

	if (bDrawDebug)
	{
		DrawDebugDirectionalArrow(GetWorld(), SweepOrigin, HitActor->GetActorLocation(), 40.f, FColor::Red, false, 1.f, 0, 2.f);
	}
}

void UGAS_GlacialCharge::StartSplineCorrection()
{
	if (!bDashInProgress)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			SplineCorrectionTimerHandle,
			this,
			&ThisClass::UpdateSplineCorrection,
			FMath::Max(0.001f, SplineCorrectionInterval),
			true);
	}
}

void UGAS_GlacialCharge::ReleaseChargeControlNearSplineEnd()
{
	if (bHasReleasedNearEndControl)
	{
		return;
	}

	bHasReleasedNearEndControl = true;

	ACharacter* Character = CachedCharacter.Get();
	if (Character == nullptr)
	{
		return;
	}

	if (UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement())
	{
		CharacterMovement->SetMovementMode(MOVE_Walking);
		bHasSavedMovementMode = false;
		SavedMovementMode = MOVE_Walking;
		SavedCustomMovementMode = 0;
	}

	UGAS_FunctionLibrary::RemoveBlockMovementTag(GetAbilitySystemComponentFromActorInfo());
}

void UGAS_GlacialCharge::UpdateSplineCorrection()
{
	if (!bDashInProgress || DashSpline == nullptr)
	{
		return;
	}

	UWorld* World = GetWorld();
	ACharacter* Character = CachedCharacter.Get();
	if (World == nullptr || Character == nullptr || DashDuration <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const float ElapsedTime = FMath::Max(0.f, World->GetTimeSeconds() - DashStartTimeSeconds);
	const float DistanceAlpha = FMath::Clamp(ElapsedTime / DashDuration, 0.f, 1.f);
	const float TargetDistance = ActiveDashDistance * DistanceAlpha;
	const float RemainingDistance = FMath::Max(0.f, ActiveDashDistance - TargetDistance);

	SnapCharacterToSplineDistance(TargetDistance, false);

	if (RemainingDistance <= FMath::Max(0.f, EndSplineReleaseDistance))
	{
		ReleaseChargeControlNearSplineEnd();
	}

	if (DistanceAlpha >= 1.f)
	{
		World->GetTimerManager().ClearTimer(SplineCorrectionTimerHandle);
	}
}

void UGAS_GlacialCharge::SnapCharacterToSplineDistance(float DistanceAlongSpline, bool bForceSnap)
{
	ACharacter* Character = CachedCharacter.Get();
	if (Character == nullptr || DashSpline == nullptr)
	{
		return;
	}

	const float ClampedDistance = FMath::Clamp(DistanceAlongSpline, 0.f, ActiveDashDistance);
	const FVector TargetLocation = DashSpline->GetLocationAtDistanceAlongSpline(ClampedDistance, ESplineCoordinateSpace::World);
	const FVector TargetDirection = DashSpline->GetDirectionAtDistanceAlongSpline(ClampedDistance, ESplineCoordinateSpace::World).GetSafeNormal();
	const float DriftDistance = FVector::Dist(Character->GetActorLocation(), TargetLocation);

	if (bForceSnap || DriftDistance > KINDA_SMALL_NUMBER)
	{
		Character->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
	}

	if (!TargetDirection.IsNearlyZero())
	{
		Character->SetActorRotation(FRotator(0.f, TargetDirection.Rotation().Yaw, 0.f));
	}

	if (bDrawDebug)
	{
		DrawDebugSphere(GetWorld(), TargetLocation, 8.f, 8, FColor::Blue, false, 0.1f);
	}
}

void UGAS_GlacialCharge::ConfigureOwnerTrailCollision(bool bEnableIgnore)
{
	UCapsuleComponent* CapsuleComponent = CachedCapsuleComponent.Get();
	if (CapsuleComponent == nullptr)
	{
		return;
	}

	for (const TWeakObjectPtr<AGAS_GlacialChargeIceBlock>& IceBlockPtr : SpawnedIceBlocks)
	{
		if (AGAS_GlacialChargeIceBlock* IceBlock = IceBlockPtr.Get())
		{
			CapsuleComponent->IgnoreActorWhenMoving(IceBlock, bEnableIgnore);
		}
	}
}

bool UGAS_GlacialCharge::IsValidChargeTarget(AActor* CandidateActor) const
{
	const ACharacter* Character = CachedCharacter.Get();
	const APawn* CandidatePawn = Cast<APawn>(CandidateActor);
	if (Character == nullptr || CandidatePawn == nullptr || CandidateActor == Character)
	{
		return false;
	}

	if (CandidatePawn->IsPlayerControlled() || !CandidateActor->Implements<UCombatInterface>())
	{
		return false;
	}

	return !ICombatInterface::Execute_IsDead(CandidateActor);
}

float UGAS_GlacialCharge::GetActorFeetZ(const AActor* Actor) const
{
	if (const ACharacter* Character = Cast<ACharacter>(Actor))
	{
		if (const UCapsuleComponent* CapsuleComponent = Character->GetCapsuleComponent())
		{
			return CapsuleComponent->GetComponentLocation().Z - CapsuleComponent->GetScaledCapsuleHalfHeight();
		}
	}

	FVector ActorOrigin = FVector::ZeroVector;
	FVector ActorExtent = FVector::ZeroVector;
	if (Actor)
	{
		Actor->GetActorBounds(true, ActorOrigin, ActorExtent);
	}

	return ActorOrigin.Z - ActorExtent.Z;
}

float UGAS_GlacialCharge::GetIceBlockWorldHalfHeight(const FTransform& BlockTransform) const
{
	const FVector HalfDimensions = IceBlockDimensions * 0.5f;
	const FVector XAxis = BlockTransform.GetRotation().GetAxisX();
	const FVector YAxis = BlockTransform.GetRotation().GetAxisY();
	const FVector ZAxis = BlockTransform.GetRotation().GetAxisZ();

	return
		(FMath::Abs(XAxis.Z) * HalfDimensions.X) +
		(FMath::Abs(YAxis.Z) * HalfDimensions.Y) +
		(FMath::Abs(ZAxis.Z) * HalfDimensions.Z);
}

bool UGAS_GlacialCharge::TraceGroundSample(ACharacter* Character, const FVector& SamplePoint, FHitResult& OutHit) const
{
	if (Character == nullptr)
	{
		return false;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(GlacialChargeGroundTrace), false, Character);
	QueryParams.AddIgnoredActor(Character);

	const FVector TraceStart = SamplePoint + FVector::UpVector * GroundTraceStartHeight;
	const FVector TraceEnd = SamplePoint - FVector::UpVector * GroundTraceDepth;
	const bool bHit = Character->GetWorld()->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

	if (bDrawDebug)
	{
		DrawDebugLine(Character->GetWorld(), TraceStart, TraceEnd, bHit ? FColor::Green : FColor::Red, false, 2.f, 0, 1.5f);
	}

	return bHit;
}

FVector UGAS_GlacialCharge::GetDashDirection(const ACharacter* Character) const
{
	if (Character == nullptr)
	{
		return FVector::ForwardVector;
	}

	if (const AController* Controller = Character->GetController())
	{
		const FRotator ControlYaw(0.f, Controller->GetControlRotation().Yaw, 0.f);
		return FRotationMatrix(ControlYaw).GetUnitAxis(EAxis::X).GetSafeNormal();
	}

	return Character->GetActorForwardVector().GetSafeNormal2D();
}

FVector UGAS_GlacialCharge::ComputeSideKnockbackDirection(const AActor* HitActor, const FVector& DashForward, float TraceAngleDegrees)
{
	const ACharacter* Character = CachedCharacter.Get();
	if (Character == nullptr || HitActor == nullptr)
	{
		return FVector::ZeroVector;
	}

	const FVector ForwardDirection = DashForward.GetSafeNormal2D();
	FVector RightDirection = FVector::CrossProduct(FVector::UpVector, ForwardDirection).GetSafeNormal();
	if (ForwardDirection.IsNearlyZero() || RightDirection.IsNearlyZero())
	{
		return FVector::ZeroVector;
	}

	const FVector ToTarget = (HitActor->GetActorLocation() - Character->GetActorLocation()).GetSafeNormal2D();
	float SideSign = FVector::DotProduct(ToTarget, RightDirection);

	if (FMath::IsNearlyZero(SideSign, 0.1f))
	{
		if (!FMath::IsNearlyZero(TraceAngleDegrees, 1.f))
		{
			SideSign = FMath::Sign(TraceAngleDegrees);
		}
		else
		{
			SideSign = bUseRightSideForCenterHit ? 1.f : -1.f;
			bUseRightSideForCenterHit = !bUseRightSideForCenterHit;
		}
	}
	else
	{
		SideSign = FMath::Sign(SideSign);
	}

	const FVector LateralDirection = RightDirection * SideSign;
	return (LateralDirection + (ForwardDirection * EnemyForwardKnockbackBlend)).GetSafeNormal();
}

FVector UGAS_GlacialCharge::SampleSurfaceNormalAtDistance(float DistanceAlongSpline) const
{
	if (DashSpline == nullptr || DashSplineNormals.IsEmpty())
	{
		return FVector::UpVector;
	}

	const float InputKey = DashSpline->GetInputKeyValueAtDistanceAlongSpline(DistanceAlongSpline);
	const int32 LowerIndex = FMath::Clamp(FMath::FloorToInt(InputKey), 0, DashSplineNormals.Num() - 1);
	const int32 UpperIndex = FMath::Clamp(FMath::CeilToInt(InputKey), 0, DashSplineNormals.Num() - 1);
	const float Alpha = FMath::Frac(InputKey);

	return FMath::Lerp(DashSplineNormals[LowerIndex], DashSplineNormals[UpperIndex], Alpha).GetSafeNormal();
}

FTransform UGAS_GlacialCharge::MakeIceBlockTransform(float DistanceAlongSpline) const
{
	const FVector SplineLocation = DashSpline->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
	const FVector Tangent = DashSpline->GetDirectionAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World).GetSafeNormal();
	const FVector SurfaceNormal = SampleSurfaceNormalAtDistance(DistanceAlongSpline);

	FVector BlockLocation = SplineLocation;
	BlockLocation.Z -= DashSplineVerticalOffset;
	BlockLocation.Z += (IceBlockDimensions.Z * 0.5f) + TrailSurfaceOffset;

	const FRotator BlockRotation = FRotationMatrix::MakeFromXZ(Tangent, SurfaceNormal).Rotator();
	return FTransform(BlockRotation, BlockLocation, FVector::OneVector);
}

void UGAS_GlacialCharge::RestoreMovementMode()
{
	ACharacter* Character = CachedCharacter.Get();
	if (!bHasSavedMovementMode || Character == nullptr)
	{
		return;
	}

	if (UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement())
	{
		CharacterMovement->SetMovementMode(SavedMovementMode, SavedCustomMovementMode);
	}

	bHasSavedMovementMode = false;
}

void UGAS_GlacialCharge::CleanupChargeState()
{
	if (bHasCleanedUp)
	{
		return;
	}

	bHasCleanedUp = true;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(IceSpawnTimerHandle);
		World->GetTimerManager().ClearTimer(ChargeHitSweepTimerHandle);
		World->GetTimerManager().ClearTimer(SplineCorrectionTimerHandle);
	}

	if (ActiveRootMotionTask)
	{
		ActiveRootMotionTask->EndTask();
		ActiveRootMotionTask = nullptr;
	}

	if (ActiveMontageTask)
	{
		ActiveMontageTask->EndTask();
		ActiveMontageTask = nullptr;
	}

	if (ActiveChargeDelayTask)
	{
		ActiveChargeDelayTask->EndTask();
		ActiveChargeDelayTask = nullptr;
	}

	bDashInProgress = false;
	RestoreMovementMode();
	ConfigureOwnerTrailCollision(false);
	UGAS_FunctionLibrary::RemoveBlockMovementTag(GetAbilitySystemComponentFromActorInfo());

	if (DashSpline)
	{
		DashSpline->DestroyComponent();
		DashSpline = nullptr;
	}
	
	DashSegments.Reset();
	DashSplineNormals.Reset();
	PendingIceBlockTransforms.Reset();
	CurrentDashSegmentIndex = INDEX_NONE;
	NextIceBlockIndex = 0;
	ActiveDashDistance = 0.f;
	DashSplineVerticalOffset = 0.f;
	EffectiveIceSpawnInterval = 0.f;
	DashStartTimeSeconds = 0.f;
	CachedDashDirection = FVector::ForwardVector;
	AffectedChargeTargets.Reset();
	SpawnedIceBlocks.Reset();
	bHasReleasedNearEndControl = false;
	bUseRightSideForCenterHit = true;
}

void UGAS_GlacialCharge::OnDashSegmentFinished()
{
	ActiveRootMotionTask = nullptr;
	++CurrentDashSegmentIndex;
	StartNextDashSegment();
}

void UGAS_GlacialCharge::OnMontageTaskFinished()
{
	ActiveMontageTask = nullptr;

	ACharacter* Character = CachedCharacter.Get();
	if (Character == nullptr)
	{
		return;
	}

	if (UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement())
	{
		CharacterMovement->SetMovementMode(MOVE_Walking);
		bHasSavedMovementMode = false;
		SavedMovementMode = MOVE_Walking;
		SavedCustomMovementMode = 0;
	}
}

void UGAS_GlacialCharge::OnChargeStartupDelayFinished()
{
	ActiveChargeDelayTask = nullptr;
	StartChargeExecution();
}
