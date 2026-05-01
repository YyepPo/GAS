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

void UMeleeDetectionComponent::StartTrace(const FName InStartSocket,const FName InEndSocket)
{
	bStartTrace = true;
	
	TraceStartSocket = InStartSocket;
	TraceEndSocket = InEndSocket;
	
	USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();

	// Seed previous positions so first frame trace is a zero-length sweep, not a wild sweep from origin
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
	
	TArray<FHitResult> HitResults;
	
	USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
    
	FVector CurrentStart = Mesh->GetSocketLocation(TraceStartSocket);
	FVector CurrentEnd = Mesh->GetSocketLocation(TraceEndSocket);
	
	FCollisionShape Shape =	FCollisionShape::MakeCapsule(32,32);
	
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	
	 World->SweepMultiByChannel(HitResults,PreviousStartLocation,CurrentStart,
		FQuat::Identity,
		ECollisionChannel::ECC_Visibility,Shape,Params);
	
	World->SweepMultiByChannel(HitResults,PreviousEndLocation,CurrentEnd,
		FQuat::Identity,
		ECollisionChannel::ECC_Visibility,Shape,Params);

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