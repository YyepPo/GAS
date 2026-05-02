#include "Components/MeleeDetectionComponent.h"

UMeleeDetectionComponent::UMeleeDetectionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

void UMeleeDetectionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UMeleeDetectionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (bStartTrace)
	{
		DoTrace();
	}
}

void UMeleeDetectionComponent::StartTrace(const FName InStartSocket,const FName InEndSocket,
	bool EnableDebug,float InCapsuleRadius,float InCapsuleHeight)
{
	bStartTrace = true;
	
	bEnableDebug = EnableDebug;
	
	TraceStartSocket = InStartSocket;
	TraceEndSocket = InEndSocket;
	
	CapsuleRadius = InCapsuleRadius;
	CapsuleHalfHeight = InCapsuleHeight;;
	
	USkeletalMeshComponent* Mesh = GetOwnersMesh();
	if (Mesh == nullptr) return;
	
	PreviousStartLocation = Mesh->GetSocketLocation(TraceStartSocket);
	PreviousEndLocation = Mesh->GetSocketLocation(TraceEndSocket);
}

void UMeleeDetectionComponent::StopTrace()
{
	bStartTrace = false;
	
	TraceStartSocket = FName("");
	TraceEndSocket = FName("");
	
	HitActors.Empty();
}

void UMeleeDetectionComponent::DoTrace()
{		
	UWorld* World =	GetWorld();
	if (World == nullptr)
	{
		return;
	}
	
	AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return;
	}
	
	USkeletalMeshComponent* Mesh = GetOwnersMesh();
	if (Mesh == nullptr)
	{
		return;
	}
	
	TArray<FHitResult> HitResults;
    
	const FVector CurrentStart = Mesh->GetSocketLocation(TraceStartSocket);
	const FVector CurrentEnd = Mesh->GetSocketLocation(TraceEndSocket);
	
	const FCollisionShape Shape = FCollisionShape::MakeCapsule(CapsuleRadius,CapsuleHalfHeight);
	
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	
	 World->SweepMultiByChannel(HitResults,PreviousStartLocation,PreviousEndLocation,
		FQuat::Identity,
		ECollisionChannel::ECC_Visibility,Shape,Params);
	
	World->SweepMultiByChannel(HitResults,CurrentStart,CurrentEnd,
		FQuat::Identity,
		ECollisionChannel::ECC_Visibility,Shape,Params);

	if(bEnableDebug)
	{
		DrawDebugCapsule(World, PreviousStartLocation, CapsuleHalfHeight, CapsuleRadius, FQuat::Identity, FColor::Green, false, 0.1f);
		DrawDebugCapsule(World, CurrentStart, CapsuleHalfHeight, CapsuleRadius, FQuat::Identity, FColor::Blue, false, 0.1f);
		DrawDebugLine(World, PreviousStartLocation, CurrentStart, FColor::Green, false, 0.1f);

		DrawDebugCapsule(World, PreviousEndLocation, CapsuleHalfHeight, CapsuleRadius, FQuat::Identity, FColor::Green, false, 0.1f);
		DrawDebugCapsule(World, CurrentEnd, CapsuleHalfHeight, CapsuleRadius, FQuat::Identity, FColor::Blue, false, 0.1f);
		DrawDebugLine(World, PreviousEndLocation, CurrentEnd, FColor::Green, false, 0.1f);
	}

	for (const FHitResult& HitResult : HitResults)
	{
		if (HitResult.bBlockingHit)
		{
			AActor* HitActor =	HitResult.GetActor();
			if (HitActor == nullptr)
			{
				continue;
			}
		
			if (HitActors.Contains(HitActor))
			{
				continue;
			}
		
			HitActors.Add(HitActor);
		
			OnHitDetected.Broadcast(HitResult);
		}
	}
	
	PreviousStartLocation = CurrentStart;
	PreviousEndLocation = CurrentEnd;
}

USkeletalMeshComponent* UMeleeDetectionComponent::GetOwnersMesh() const
{
	USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
	if (Mesh == nullptr)
	{
		UE_LOG(LogTemp,Warning,TEXT("UMeleeDetectionComponent: Start Trace: Owner's mesh component is not valid %s "), *GetOwner()->GetActorNameOrLabel())
		return nullptr;
	}
	return Mesh;
}
