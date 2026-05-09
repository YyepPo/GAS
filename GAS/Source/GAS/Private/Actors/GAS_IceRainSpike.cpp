#include "Actors/GAS_IceRainSpike.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilityComponent/GAS_FunctionLibrary.h"
#include "Components/SphereComponent.h"
#include "Data/GAS_AbilityTypes.h"
#include "GAS_GameplayTags.h"
#include "GameFramework/ProjectileMovementComponent.h"

AGAS_IceRainSpike::AGAS_IceRainSpike()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->SetSphereRadius(35.f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionSphere->SetNotifyRigidBodyCollision(true);

	SpikeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpikeMesh"));
	SpikeMesh->SetupAttachment(CollisionSphere);
	SpikeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionSphere;
	ProjectileMovement->InitialSpeed = 2200.f;
	ProjectileMovement->MaxSpeed = 2200.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
}

void AGAS_IceRainSpike::InitializeSpike(UAbilitySystemComponent* InSourceAbilitySystemComponent,
	TSubclassOf<UGameplayEffect> InDamageEffectClass, TSubclassOf<UGameplayEffect> InStunEffectClass,
	float InAbilityLevel, float InBaseDamage, FGameplayTag InDamageType, const FVector& InVelocity)
{
	SourceAbilitySystemComponent = InSourceAbilitySystemComponent;
	DamageEffectClass = InDamageEffectClass;
	StunEffectClass = InStunEffectClass;
	AbilityLevel = InAbilityLevel;
	BaseDamage = InBaseDamage;
	DamageType = InDamageType;

	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = InVelocity;
		ProjectileMovement->InitialSpeed = InVelocity.Size();
		ProjectileMovement->MaxSpeed = InVelocity.Size();
	}
}

void AGAS_IceRainSpike::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(Lifetime);
	CollisionSphere->OnComponentHit.AddDynamic(this, &ThisClass::OnSpikeHit);
}

void AGAS_IceRainSpike::OnSpikeHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority() || bHasImpacted)
	{
		return;
	}

	bHasImpacted = true;

	if (OtherActor && OtherActor != GetOwner() && OtherActor != GetInstigator())
	{
		ApplyEffectsToTarget(OtherActor, Hit);
	}

	Destroy();
}

void AGAS_IceRainSpike::ApplyEffectsToTarget(AActor* TargetActor, const FHitResult& Hit)
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	UAbilitySystemComponent* SourceASC = SourceAbilitySystemComponent;
	if (SourceASC == nullptr)
	{
		SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
	}

	if (TargetASC == nullptr || SourceASC == nullptr)
	{
		return;
	}

	const FGameplayTag EffectiveDamageType = DamageType.IsValid()
		? DamageType
		: FGAS_GameplayTags::Get().Damage_Physical;

	if (DamageEffectClass)
	{
		FDamageEffectParams DamageParams;
		DamageParams.WorldContextObject = this;
		DamageParams.DamageGameplayEffectClass = DamageEffectClass;
		DamageParams.SourceAbilitySystemComponent = SourceASC;
		DamageParams.TargetAbilitySystemComponent = TargetASC;
		DamageParams.BaseDamage = BaseDamage;
		DamageParams.AbilityLevel = AbilityLevel;
		DamageParams.DamageType = EffectiveDamageType;

		UGAS_FunctionLibrary::ApplyDamageEffect(DamageParams);
	}

	if (StunEffectClass)
	{
		FGameplayEffectContextHandle StunContext = SourceASC->MakeEffectContext();
		StunContext.AddSourceObject(this);
		StunContext.AddHitResult(Hit);

		const FGameplayEffectSpecHandle StunSpec = SourceASC->MakeOutgoingSpec(
			StunEffectClass,
			AbilityLevel,
			StunContext);

		if (StunSpec.Data.IsValid())
		{
			SourceASC->ApplyGameplayEffectSpecToTarget(*StunSpec.Data.Get(), TargetASC);
		}
	}
}
