#include "Abilities/ChargeAbility/GAS_GlacialChargeIceBlock.h"

#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

AGAS_GlacialChargeIceBlock::AGAS_GlacialChargeIceBlock()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	IceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IceMesh"));
	IceMesh->SetupAttachment(SceneRoot);
	IceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	IceMesh->SetGenerateOverlapEvents(false);

	CollisionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionVolume"));
	CollisionVolume->SetupAttachment(SceneRoot);
	CollisionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionVolume->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);

	InitializeIceBlock(BlockDimensions);
}

void AGAS_GlacialChargeIceBlock::InitializeIceBlock(const FVector& InDimensions, AActor* InIgnoredActor)
{
	BlockDimensions = InDimensions.ComponentMax(FVector(10.f, 10.f, 10.f));
	IgnoredActor = InIgnoredActor;

	const FVector HalfExtents = (BlockDimensions * 0.5f) - FVector(CollisionPadding);
	CollisionVolume->SetBoxExtent(HalfExtents.ComponentMax(FVector(1.f, 1.f, 1.f)));
	//IceMesh->SetRelativeScale3D(FVector(BlockDimensions.X / 100.f, BlockDimensions.Y / 100.f, BlockDimensions.Z / 100.f));

	ConfigureCollisionForDashOwner();
}

void AGAS_GlacialChargeIceBlock::BeginPlay()
{
	Super::BeginPlay();

	ConfigureCollisionForDashOwner();
	//SetLifeSpan(LifeSeconds);
}

void AGAS_GlacialChargeIceBlock::ConfigureCollisionForDashOwner() const
{
	TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents;
	GetComponents(PrimitiveComponents);

	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (PrimitiveComponent == nullptr)
		{
			continue;
		}

		PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PrimitiveComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
		PrimitiveComponent->SetGenerateOverlapEvents(false);

		if (IgnoredActor.IsValid())
		{
			PrimitiveComponent->IgnoreActorWhenMoving(IgnoredActor.Get(), true);
		}
	}
}
