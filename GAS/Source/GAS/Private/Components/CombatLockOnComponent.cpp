#include "Components/CombatLockOnComponent.h"

#include "Camera/CameraComponent.h"
#include "Character/GAS_AuroraCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Interface/CombatInterface.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values for this component's properties
UCombatLockOnComponent::UCombatLockOnComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

void UCombatLockOnComponent::BeginPlay()
{
	Super::BeginPlay();
	
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	
	// Ignore self
	ActorsToIgnore.Add(GetOwner());

	AGAS_AuroraCharacter* Character = GetOwnerCharacter();
	if (Character)
	{
		CachedSpringArm = Character->GetSpringArmComponent(); 
		CachedCamera = Character->GetCameraComponent();
	}
	
}

void UCombatLockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	/*
	FString CurrentTargetNameMsg = FString::Printf(TEXT("LockOnComponent: CurrentTarget name: %s"), 
	CurrentTarget ? *CurrentTarget->GetActorNameOrLabel() : TEXT("None"));
	GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Green, CurrentTargetNameMsg);

	DrawDebugSphere(GetWorld(),GetOwner()->GetActorLocation(),SphereRadius,14,FColor::Green);

	if (CurrentTarget)
	{
		DrawDebugSphere(GetWorld(),CurrentTarget->GetActorLocation(),120,16,FColor::Blue);
	}*/

	if (CurrentTarget && bLockStarted)
	{
		bIsRestoringFromLock = false;

		FVector TargetLocation = CurrentTarget->GetActorLocation();
		TargetLocation.Z += 0.f;

		FVector Direction = TargetLocation - GetOwner()->GetActorLocation();
		FRotator LookAtRotation = FRotationMatrix::MakeFromX(Direction).Rotator();

		// Smooth interpolation
		FRotator CurrentRot = CachedSpringArm->GetComponentRotation();
		FRotator NewRot = FMath::RInterpTo(CurrentRot, LookAtRotation, DeltaTime, SpringRotateInterpSpeed);
		CachedSpringArm->SetWorldRotation(NewRot);

		FVector cl = CachedSpringArm->GetRelativeLocation();
		FVector nw = FMath::VInterpTo(cl, SpringArmOffsetOnLock, DeltaTime,SpringLocationInterpSpeed);
		CachedSpringArm->SetRelativeLocation(nw);
		
		// Rotate player mesh towards the target
		FRotator CurrentRotation = GetOwner()->GetActorRotation();
		FRotator TargetRot = FMath::RInterpTo(CurrentRotation, LookAtRotation, DeltaTime, MeshRotateInterSpeed);
		GetOwner()->SetActorRotation(FRotator(GetOwner()->GetActorRotation().Pitch,TargetRot.Yaw,GetOwner()->GetActorRotation().Roll));
	}
	else if (bIsRestoringFromLock)
	{
		bool bArmRotDone  = false;
		bool bArmLocDone  = false;
		bool bActorRotDone = false;

		// Restore spring arm rotation
		FRotator CurrentArmRot = CachedSpringArm->GetComponentRotation();
		FRotator TargetArmRot = FRotator(
			PreLockSpringArmRotation.Pitch,
			CachedSpringArm->GetComponentRotation().Yaw,
			PreLockSpringArmRotation.Roll);
		FRotator NewArmRot = FMath::RInterpTo(CurrentArmRot, TargetArmRot, DeltaTime, SpringRotateInterpSpeed);
		bArmRotDone = NewArmRot.Equals(PreLockSpringArmRotation, 0.5f);
		
		// Restore spring arm location
		FVector CurrentArmLoc = CachedSpringArm->GetRelativeLocation();
		FVector NewArmLoc = FMath::VInterpTo(CurrentArmLoc, PreLockSpringArmLocation, DeltaTime, SpringLocationInterpSpeed);
		CachedSpringArm->SetRelativeLocation(NewArmLoc);
		bArmLocDone = NewArmLoc.Equals(PreLockSpringArmLocation, 1.f);

		// Stop restoring once everything has snapped back
		if (bArmRotDone && bArmLocDone && bActorRotDone)
		{
			bIsRestoringFromLock = false;
		}
	}
	
}

void UCombatLockOnComponent::StartLock(UCameraComponent* CameraComponent)
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}
	
	AActor* OwnerActor = GetOwnerActor();
	if (OwnerActor == nullptr)
	{
		return;
	}

	CachedCamera = CameraComponent;

	SavePreLockState();
	
	TArray<AActor*> ValidTargets = GetLockableTargets();
	if (ValidTargets.IsEmpty())
	{
		return;
	}
	
	LockToTarget(SelectTargetClosestToMiddleOfTheScreen(ValidTargets));
}

bool UCombatLockOnComponent::IsTargetInFront(AActor* Target,const FVector& CameraLocation,const FVector& CameraForwardVector)
{
	if (Target == nullptr)
	{
		return false;
	}
	
	// Calculate direction to the lockable actor
	const FVector Direction = (Target->GetActorLocation() - CameraLocation).GetSafeNormal(0.00);
	float Dot =	FVector::DotProduct(CameraForwardVector,Direction);
		
	return Dot > 0.5f;
}

bool UCombatLockOnComponent::IsTargetVisible(AActor* Target, const FVector& CameraLocation)
{
	UWorld* World =	GetWorld();
	if (World == nullptr)
	{
		return false;
	}
	
	if (Target == nullptr)
	{
		return false;
	}
	// Make a line trace towards the target from the camera
	
	const FVector LineStartLocation = CameraLocation;
	ACharacter* TargetCharacter = Cast<ACharacter>(Target);
	if (TargetCharacter == nullptr)
	{
		return false;
	}
	const FVector LineEndLocation = Target->GetActorLocation();// + FVector(0,0,TargetCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()) ;
	
	FHitResult HitResult;
	World->LineTraceSingleByChannel(HitResult,
		LineStartLocation,
		LineEndLocation,ECC_Visibility);
	
	DrawDebugLine(World,LineStartLocation,LineEndLocation,FColor::Red,true);

	return HitResult.bBlockingHit == true && Target == HitResult.GetActor();
}

void UCombatLockOnComponent::LockToTarget(AActor* Target)
{
	ListenForTargetDeathEvent(Target);
	CurrentTarget = Target;
	
	bLockStarted = true;
	
	OnLockTargetUpdated.Broadcast(Target);
}

AActor* UCombatLockOnComponent::SelectTargetClosestToMiddleOfTheScreen(const TArray<AActor*>& Actors)
{
	if (Actors.Num() == 0)
	{
		return nullptr;
	}

	APlayerController* PlayerController = GetPlayerController();
	if (!PlayerController)
	{
		return nullptr;
	}

	int32 ScreenX, ScreenY;
	PlayerController->GetViewportSize(ScreenX, ScreenY);
	const FVector2D ScreenCenter(ScreenX * 0.5f, ScreenY * 0.5f);

	AActor* BestTarget = nullptr;
	float BestDistanceSq = FLT_MAX;

	for (AActor* Target : Actors)
	{
		FVector2D ScreenPos = FVector2D::ZeroVector;
		if (!TryProjectTargetToScreen(Target, ScreenPos))
		{
			continue;
		}

		const float DistSq = FVector2D::DistSquared(ScreenPos, ScreenCenter);

		if (DistSq < BestDistanceSq)
		{
			BestDistanceSq = DistSq;
			BestTarget = Target;
		}
	}

	return BestTarget;
}

AActor* UCombatLockOnComponent::SelectNextTarget(const TArray<AActor*>& Actors,
	ELockOnSwitchDirection Direction) const
{
	if (CurrentTarget == nullptr)
	{
		return nullptr;
	}

	FVector2D CurrentTargetScreenPosition = FVector2D::ZeroVector;
	if (!TryProjectTargetToScreen(CurrentTarget, CurrentTargetScreenPosition))
	{
		return nullptr;
	}

	AActor* BestCandidate = nullptr;
	float BestHorizontalDelta = FLT_MAX;
	float BestDistanceSq = FLT_MAX;

	AActor* WrapCandidate = nullptr;
	float WrapScreenX = Direction == ELockOnSwitchDirection::Right ? FLT_MAX : -FLT_MAX;
	float WrapDistanceSq = FLT_MAX;

	for (AActor* Target : Actors)
	{
		if (Target == nullptr || Target == CurrentTarget || !IsTargetAlive(Target))
		{
			continue;
		}

		FVector2D CandidateScreenPosition = FVector2D::ZeroVector;
		if (!TryProjectTargetToScreen(Target, CandidateScreenPosition))
		{
			continue;
		}

		const float DeltaX = CandidateScreenPosition.X - CurrentTargetScreenPosition.X;
		const bool bIsRequestedDirection =
			Direction == ELockOnSwitchDirection::Right ? DeltaX > KINDA_SMALL_NUMBER : DeltaX < -KINDA_SMALL_NUMBER;
		const float HorizontalDelta = FMath::Abs(DeltaX);
		const float ScreenDistanceSq = FVector2D::DistSquared(CandidateScreenPosition, CurrentTargetScreenPosition);

		if (bIsRequestedDirection)
		{
			const bool bIsBetterCandidate =
				HorizontalDelta < BestHorizontalDelta ||
				(FMath::IsNearlyEqual(HorizontalDelta, BestHorizontalDelta) && ScreenDistanceSq < BestDistanceSq);

			if (bIsBetterCandidate)
			{
				BestCandidate = Target;
				BestHorizontalDelta = HorizontalDelta;
				BestDistanceSq = ScreenDistanceSq;
			}

			continue;
		}

		if (!bWrapSwitchTarget)
		{
			continue;
		}

		if (Direction == ELockOnSwitchDirection::Right)
		{
			const bool bIsBetterWrapCandidate =
				CandidateScreenPosition.X < WrapScreenX ||
				(FMath::IsNearlyEqual(CandidateScreenPosition.X, WrapScreenX) && ScreenDistanceSq < WrapDistanceSq);

			if (bIsBetterWrapCandidate)
			{
				WrapCandidate = Target;
				WrapScreenX = CandidateScreenPosition.X;
				WrapDistanceSq = ScreenDistanceSq;
			}
		}
		else
		{
			const bool bIsBetterWrapCandidate =
				CandidateScreenPosition.X > WrapScreenX ||
				(FMath::IsNearlyEqual(CandidateScreenPosition.X, WrapScreenX) && ScreenDistanceSq < WrapDistanceSq);

			if (bIsBetterWrapCandidate)
			{
				WrapCandidate = Target;
				WrapScreenX = CandidateScreenPosition.X;
				WrapDistanceSq = ScreenDistanceSq;
			}
		}
	}

	return BestCandidate != nullptr ? BestCandidate : WrapCandidate;
}

bool UCombatLockOnComponent::TryProjectTargetToScreen(AActor* Target, FVector2D& OutScreenPosition) const
{
	OutScreenPosition = FVector2D::ZeroVector;

	if (!IsTargetAlive(Target))
	{
		return false;
	}

	APlayerController* PlayerController = GetPlayerController();
	if (PlayerController == nullptr)
	{
		return false;
	}

	int32 ViewportX = 0;
	int32 ViewportY = 0;
	PlayerController->GetViewportSize(ViewportX, ViewportY);
	if (ViewportX <= 0 || ViewportY <= 0)
	{
		return false;
	}

	FVector CameraLocation = FVector::ZeroVector;
	FVector CameraForwardVector = FVector::ForwardVector;
	if (CachedCamera != nullptr)
	{
		CameraLocation = CachedCamera->GetComponentLocation();
		CameraForwardVector = CachedCamera->GetForwardVector();
	}
	else if (PlayerController->PlayerCameraManager != nullptr)
	{
		CameraLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
		CameraForwardVector = PlayerController->PlayerCameraManager->GetCameraRotation().Vector();
	}
	else
	{
		return false;
	}

	FVector ProjectionLocation = Target->GetActorLocation();
	FVector BoundsExtent = FVector::ZeroVector;
	Target->GetActorBounds(true, ProjectionLocation, BoundsExtent);

	const FVector DirectionToTarget = (ProjectionLocation - CameraLocation).GetSafeNormal();
	if (FVector::DotProduct(CameraForwardVector, DirectionToTarget) <= 0.f)
	{
		return false;
	}

	if (!PlayerController->ProjectWorldLocationToScreen(ProjectionLocation, OutScreenPosition, true))
	{
		return false;
	}

	return OutScreenPosition.X >= 0.f &&
		OutScreenPosition.X <= static_cast<float>(ViewportX) &&
		OutScreenPosition.Y >= 0.f &&
		OutScreenPosition.Y <= static_cast<float>(ViewportY);
}

bool UCombatLockOnComponent::IsTargetAlive(AActor* Target) const
{
	if (Target == nullptr || !Target->GetClass()->ImplementsInterface(UCombatInterface::StaticClass()))
	{
		return false;
	}

	return !ICombatInterface::Execute_IsDead(Target);
}

TArray<AActor*> UCombatLockOnComponent::GetLockableTargets()
{
	AActor* OwnerActor = GetOwnerActor();
	if (OwnerActor == nullptr || CachedCamera == nullptr)
	{
		return TArray<AActor*>();
	}
	LockableTargets.Empty();
	
	TArray<AActor*> OverlappedActors;

	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),                    
		OwnerActor->GetActorLocation(),
		SphereRadius,                  
		ObjectTypes,                   
		ActorClassFilter,              
		ActorsToIgnore,                
		OverlappedActors          
	);

	TArray<AActor*> ValidTargets;
	
	for (AActor* Target : OverlappedActors)
	{
		if (!IsTargetAlive(Target))
		{
			continue;
		}
		
		if (IsTargetInFront(Target,CachedCamera->GetComponentLocation(),CachedCamera->GetForwardVector()) == false)
		{
			continue;
		}
		
		if (IsTargetVisible(Target,CachedCamera->GetComponentLocation()) == false)
		{
			continue;
		}
		
		ValidTargets.Add(Target);		
	}
	return ValidTargets;
}

void UCombatLockOnComponent::ListenForTargetDeathEvent(AActor* Target)
{
	// Unbind from the old target 
	if (CurrentTarget)
	{
		if (ICombatInterface* OldEnemy = Cast<ICombatInterface>(CurrentTarget))
		{
			OldEnemy->GetOnDeathDelegate().RemoveDynamic(this,&UCombatLockOnComponent::OnTargetDeathEvent);
		}
	}	
	
	// Bind to the new target
	if (Target)
	{
		if (ICombatInterface* NewEnemy = Cast<ICombatInterface>(Target))
		{
			NewEnemy->GetOnDeathDelegate().AddDynamic(this,&UCombatLockOnComponent::OnTargetDeathEvent);
		}
	}
}

void UCombatLockOnComponent::OnTargetDeathEvent(AActor* Target)
{
	TArray<AActor*> Actors = GetLockableTargets();
	if (Actors.IsEmpty())
	{
		StopLock();
		return;
	}
	
	LockToTarget(SelectTargetClosestToMiddleOfTheScreen(Actors));
}

void UCombatLockOnComponent::SwitchTarget()
{
	SwitchTargetRight();
}

void UCombatLockOnComponent::SwitchTargetLeft()
{
	if (CurrentTarget == nullptr)
	{
		return;
	}

	const TArray<AActor*> ValidTargets = GetLockableTargets();
	if (ValidTargets.IsEmpty())
	{
		return;
	}

	if (AActor* NextTarget = SelectNextTarget(ValidTargets, ELockOnSwitchDirection::Left))
	{
		LockToTarget(NextTarget);
	}
}

void UCombatLockOnComponent::SwitchTargetRight()
{
	if (CurrentTarget == nullptr)
	{
		return;
	}

	const TArray<AActor*> ValidTargets = GetLockableTargets();
	if (ValidTargets.IsEmpty())
	{
		return;
	}

	if (AActor* NextTarget = SelectNextTarget(ValidTargets, ELockOnSwitchDirection::Right))
	{
		LockToTarget(NextTarget);
	}
}

void UCombatLockOnComponent::StopLock()
{
	// Unbind
	if (CurrentTarget)
	{
		if (ICombatInterface* OldEnemy = Cast<ICombatInterface>(CurrentTarget))
		{
			OldEnemy->GetOnDeathDelegate().RemoveDynamic(this,&UCombatLockOnComponent::OnTargetDeathEvent);
		}
	}
	
	bLockStarted = false;
	CurrentTarget = nullptr;
	
	LockableTargets.Empty();
	
	OnLockTargetUpdated.Broadcast(nullptr);

	bIsRestoringFromLock = true;
	
	if (AGAS_AuroraCharacter* AuroraCharacter =	GetOwnerCharacter())
	{
		AuroraCharacter->GetSpringArmComponent()->bUsePawnControlRotation = true;
		AuroraCharacter->GetCharacterMovement()->bUseControllerDesiredRotation = true;	
	}
	
}

bool UCombatLockOnComponent::CanLock() const
{
	return LockableTargets.Num() > 0;
}

AActor* UCombatLockOnComponent::GetOwnerActor() const
{
	if (AActor* Owner = GetOwner())
	{
		return Owner;
	}
	return nullptr;
}

APlayerController* UCombatLockOnComponent::GetPlayerController() const
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn == nullptr)
	{
		return nullptr;
	}
	
	APlayerController* OwnerController = Cast<APlayerController>(Pawn->GetController());
	if (OwnerController == nullptr)
	{
		return nullptr;
	}
	
	return OwnerController;
}

void UCombatLockOnComponent::SavePreLockState()
{
	if (AGAS_AuroraCharacter* AuroraCharacter =	GetOwnerCharacter())
	{
		PreLockSpringArmRotation = AuroraCharacter->GetSpringArmComponent()->GetComponentRotation();
		PreLockSpringArmLocation = AuroraCharacter->GetSpringArmComponent()->GetRelativeLocation();
		PreLockActorRotation     = AuroraCharacter->GetActorRotation();
		AuroraCharacter->GetSpringArmComponent()->bUsePawnControlRotation = false;
		AuroraCharacter->GetCharacterMovement()->bUseControllerDesiredRotation = false;
	}	
}

AGAS_AuroraCharacter* UCombatLockOnComponent::GetOwnerCharacter() const
{
	if (AGAS_AuroraCharacter* Character = Cast<AGAS_AuroraCharacter>(GetOwner()))
	{
		return Character;
	}

	return nullptr;
}
