#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GAS_BaseCharacter.generated.h"

class UAbilitySystemComponent;

UCLASS()
class GAS_API AGAS_BaseCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGAS_BaseCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	/** Called from server, initializes AbilitySystemComponent on the server */
	virtual void PossessedBy(AController* NewController) override;

	/** Called on client, initializes AbilitySystemComponent on the client */
	virtual void OnRep_PlayerState() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="GAS", meta=(AllowPrivateAccess="true"))
	UAbilitySystemComponent* AbilitySystemComponent;
};
