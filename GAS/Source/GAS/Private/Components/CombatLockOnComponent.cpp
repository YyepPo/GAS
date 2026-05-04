#include "Components/CombatLockOnComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Interface/CharacterInterface.h"
#include "Interface/CombatInterface.h"

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
}

void UCombatLockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	FString CurrentTargetNameMsg = FString::Printf(TEXT("LockOnComponent: CurrentTarget name: %s"), 
	CurrentTarget ? *CurrentTarget->GetActorNameOrLabel() : TEXT("None"));
	GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Green, CurrentTargetNameMsg);

	DrawDebugSphere(GetWorld(),GetOwner()->GetActorLocation(),SphereRadius,14,FColor::Green);

	if (CurrentTarget)
	{
		DrawDebugSphere(GetWorld(),CurrentTarget->GetActorLocation(),120,16,FColor::Blue);
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
		if (!Target)
		{
			continue;
		}

		FVector2D ScreenPos;
		const bool bProjected = PlayerController->ProjectWorldLocationToScreen(
			Target->GetActorLocation(),
			ScreenPos,
			true
		);

		if (!bProjected)
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

TArray<AActor*> UCombatLockOnComponent::GetLockableTargets()
{
	AActor* OwnerActor = GetOwnerActor();
	if (OwnerActor == nullptr)
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
		if (Target == nullptr)
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
	if (LockableTargets.IsEmpty())
	{
		StopLock();
		return;
	}
	
	LockToTarget(SelectTargetClosestToMiddleOfTheScreen(LockableTargets));
}

void UCombatLockOnComponent::SwitchTarget()
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

	if (CurrentTarget == nullptr)
	{
		return;
	}
	
	TArray<AActor*> ValidTargets = GetLockableTargets();
	if (ValidTargets.IsEmpty())
	{
		return;
	}
	
	APlayerController* PlayerController = GetPlayerController();
	if (PlayerController == nullptr)
	{
		return;
	}
	
	// Project current target to screen space
	FVector2D CurrentTargetScreenLoc;
	bool bCurrentTargetProjected = PlayerController->ProjectWorldLocationToScreen(CurrentTarget->GetActorLocation(),CurrentTargetScreenLoc,true);
	if (bCurrentTargetProjected == false)
	{
		return;
	}
	
	AActor* BestCandidate = nullptr;
	float BestScore = FLT_MAX;

	// project all targets to screen space
	for (AActor* Target : ValidTargets)
	{
		if (Target == nullptr)
		{
			continue;
		}

		if (CurrentTarget == Target)
		{
			continue;
		}
		
		FVector2D TargetScreenLoc = FVector2D::Zero();
		bool bTargetProjected =	PlayerController->ProjectWorldLocationToScreen(Target->GetActorLocation(),TargetScreenLoc,true);
		if (bTargetProjected == false)
		{
			UE_LOG(LogTemp,Warning,TEXT("CombatLockOnComponent: Switchtarget: Target's %s PROJECTED FALSE "),*Target->GetActorNameOrLabel());
			continue;
		}
		
		const float ScreenXOffset = TargetScreenLoc.X - CurrentTargetScreenLoc.X;
		
		UE_LOG(LogTemp,Warning,TEXT("CombatLockOnComponent: Switchtarget: Target's %s lock score is: %f "),*Target->GetActorNameOrLabel(),FMath::Abs(ScreenXOffset));

		float Score = FMath::Abs(ScreenXOffset);
		if (Score < BestScore)
		{
			BestScore = Score;
			BestCandidate = Target;
		}
	}
	
	if (BestCandidate)
	{
		LockToTarget(BestCandidate);
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
