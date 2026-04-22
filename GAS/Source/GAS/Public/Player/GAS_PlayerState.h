#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "GAS_PlayerState.generated.h"

class ULevelUpConfig;
class UAbilitySystemComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChanged, int32 /*StatValue*/)
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLevelChanged, int32 /*StatValue*/, bool /*bLevelUp*/)

UCLASS()
class GAS_API AGAS_PlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

	AGAS_PlayerState();

protected:
	
	virtual void BeginPlay() override;	

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
public:

	/** IAbilitySystemInterface function */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	/** IAbilitySystemInterface END */
	
	UAttributeSet* GetAttributeSet() const {return AttributeBase;}
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	TObjectPtr<ULevelUpConfig> LevelUpConfig;
	
	void AddToXP(int32 InXP);
	void AddToLevel(int32 InLevel);
	void AddToAttributePoints(int32 InPoints);

	///
	// Setters
	///
	void SetXP(int32 InXP);
	void SetLevel(int32 InLevel);
	void SetAttributePoints(int32 InPoints);
	
	
	/// 
	// Getters
	/// 

	UFUNCTION(BlueprintCallable,BlueprintPure, Category = "Level")
	FORCEINLINE int32 GetPlayerLevel() const {return Level;}
	
	UFUNCTION(BlueprintCallable,BlueprintPure, Category = "Level")
	FORCEINLINE int32 GetXP() const {return XP;}
	
	UFUNCTION(BlueprintCallable,BlueprintPure, Category = "Level")
	FORCEINLINE int32 GetAttributePoints() const {return AttributePoints;}
	
	
	FOnPlayerStatChanged OnXPChangedDelegate;
	FOnLevelChanged OnLevelChangedDelegate;
	FOnPlayerStatChanged OnAttributePointsChangedDelegate;
	
protected:
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeBase;
	
private:
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_Level)
	int32 Level = 1;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_XP)
	int32 XP = 0;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_AttributePoints)
	int32 AttributePoints = 0;
	
	UFUNCTION()
	void OnRep_Level(int32 OldLevel);

	UFUNCTION()
	void OnRep_XP(int32 OldXP);

	UFUNCTION()
	void OnRep_AttributePoints(int32 OldAttributePoints);

};


