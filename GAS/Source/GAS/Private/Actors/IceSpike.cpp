#include "Actors/IceSpike.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"

AIceSpike::AIceSpike()
{
	PrimaryActorTick.bCanEverTick = false;

	SpikeMesh = CreateDefaultSubobject<UStaticMeshComponent>("SpikeMesh");
	RootComponent = SpikeMesh;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>("CollisionSphere");
	CollisionSphere->SetupAttachment(RootComponent);
	CollisionSphere->SetSphereRadius(50.f);
}



void AIceSpike::BeginPlay()
{
	Super::BeginPlay();

	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AIceSpike::OnOverlapBegin);
}

void AIceSpike::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == GetInstigator()) return;

	UAbilitySystemComponent* TargetASC = 
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	UAbilitySystemComponent* InstigatorASC = 
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());

	if (TargetASC && InstigatorASC)
	{
		FGameplayEffectContextHandle Context = InstigatorASC->MakeEffectContext();
		Context.AddSourceObject(this);

		// Apply damage
		FGameplayEffectSpecHandle DamageSpec = InstigatorASC->MakeOutgoingSpec(
			DamageEffectClass, AbilityLevel, Context);
		InstigatorASC->ApplyGameplayEffectSpecToTarget(*DamageSpec.Data.Get(), TargetASC);

	
		//FGameplayEffectSpecHandle FreezeSpec = InstigatorASC->MakeOutgoingSpec(
		//	FreezeEffectClass, AbilityLevel, Context);
		//InstigatorASC->ApplyGameplayEffectSpecToTarget(*FreezeSpec.Data.Get(), TargetASC);
	
		Destroy();
	}
}

