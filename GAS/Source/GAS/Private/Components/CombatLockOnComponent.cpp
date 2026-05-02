#include "Components/CombatLockOnComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

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
	
	CachedCamera = CameraComponent;
	
	const FVector ForwardVector = CameraComponent->GetForwardVector();
	const FVector CameraLocation = CameraComponent->GetComponentLocation();
	
	// Find targets that are in front of the camera
	for (AActor* Target : OverlappedActors)
	{
		const bool bIsInFront =	IsTargetInFront(Target,CameraLocation,ForwardVector);
		if (bIsInFront)
		{
			LockableTargets.Add(Target);
			DrawDebugSphere(World,Target->GetActorLocation(),120,5,FColor::Green,true,5.f);
			continue;
		}
		
		DrawDebugSphere(World,Target->GetActorLocation(),100,5,FColor::Red,true,5.f);
	}
	
	GEngine->AddOnScreenDebugMessage(1, 10.f, FColor::Green,
		FString::Printf(TEXT("LockOnComponent: Number of lockable targets : %d"),LockableTargets.Num()));

	// Check if the target is visible to the camera, is not behind any walls or something
	for (AActor* Target : LockableTargets)
	{
		const bool bIsVisible =	IsTargetVisible(Target,CameraLocation);
		if (bIsVisible)
		{
			LockToTarget(Target);
			break;
		}
	}
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
	const FVector LineEndLocation = Target->GetActorLocation() + FVector(0,0,TargetCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()) ;
	
	FHitResult HitResult;
	World->LineTraceSingleByChannel(HitResult,
		LineStartLocation,
		LineEndLocation,ECC_Visibility);
	
	DrawDebugLine(World,LineStartLocation,LineEndLocation,FColor::Red,true);

	return HitResult.bBlockingHit == true && Target == HitResult.GetActor();
}

void UCombatLockOnComponent::LockToTarget(AActor* Target)
{
	CurrentTarget = Target;
	bLockStarted = true;
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
	
	if (ValidTargets.IsEmpty())
	{
		return;
	}
	
	APlayerController* PlayerController = GetPlayerController();
	if (PlayerController == nullptr)
	{
		return;
	}
	
	if (CurrentTarget == nullptr)
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
		FVector2D TargetScreenLoc = FVector2D::Zero();
		bool bTargetProjected =	PlayerController->ProjectWorldLocationToScreen(Target->GetActorLocation(),TargetScreenLoc,true);
		if (bTargetProjected == false)
		{
			continue;
		}
		
		const float ScreenXOffset = TargetScreenLoc.X - CurrentTargetScreenLoc.X;
		
		if (bSwitchRight && ScreenXOffset <= 0.f)
		{
			continue;
		}
		
		if (!bSwitchRight && ScreenXOffset >= 0.f)
		{
			continue;
		}

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
	bLockStarted = false;
	CurrentTarget = nullptr;
	LockableTargets.Empty();
}

bool UCombatLockOnComponent::CanLock() const
{
	return LockableTargets.Num() > 0;
}

AActor* UCombatLockOnComponent::GetOwnerActor() const
{
	if (AActor* Owner = GetOwnerActor())
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
