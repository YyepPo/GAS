#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Interface/CombatInterface.h"
#include "GAS_BaseCharacter.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;

UCLASS()
class GAS_API AGAS_BaseCharacter : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGAS_BaseCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** ICombatInterface Functions */
	virtual UAnimMontage* GetHitReactMontage_Implementation(const FGameplayTag& HitReactionTag) override;
	virtual ECharacterClass GetCharacterClass_Implementation() override;
	virtual void Die(const FVector& DeathImpulse) override;
	virtual FOnDeath& GetOnDeathDelegate() override;
	virtual FOnDamageSignature& GetOnDamageSignature() override; 
	virtual bool IsDead_Implementation() const override;
	virtual AActor* GetAvatar_Implementation() override;
	/** ICombatInterface END*/
	
	FOnDeath OnDeath;
	FOnDamageSignature OnDamageSignature;
	
	void SetCharacterClass(ECharacterClass InCharacterClass) {CharacterClass = InCharacterClass;}

	UFUNCTION(BlueprintCallable,BlueprintPure)
	UAttributeSet* GetAttributeSet() const {return AttributeSet;}
	
protected:

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	bool bIsDead = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	USoundBase* DeathSound;
	
	///
	// Hit react montages ,4 sides
	///
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	UAnimMontage* FrontHitReactMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	UAnimMontage* BackHitReactMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	UAnimMontage* LeftHitReactMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	UAnimMontage* RightHitReactMontage;
	
	
private:
	

	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class")
	ECharacterClass CharacterClass;

};
