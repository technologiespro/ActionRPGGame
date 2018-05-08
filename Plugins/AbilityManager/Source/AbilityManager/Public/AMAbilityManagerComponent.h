// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"

#include "AMTypes.h"
#include "GameplayTags.h"
#include "Abilities/GAAbilityBase.h"
#include "AMAbilityManagerComponent.generated.h"



/*
- Group
- Set
- Ability
-Inputs (multiple);

*/

//all the inputs assigned to SINGLE ability;

USTRUCT()
struct FAMAbilityInputSlot
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
		TArray<FGameplayTag> Inputs;

	TArray<FGameplayTag> operator()()
	{
		return Inputs;
	}
};


USTRUCT()
struct FAMAbilityInputGroup
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
		TArray<FAMAbilityInputSlot> Slots;
};


USTRUCT()
struct FAMAbilityInputContainer
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
		TArray<FAMAbilityInputGroup> Groups;


	TArray<FGameplayTag> GetInputs(EAMGroup InGroup, EAMSlot InSlot)
	{
		return Groups[AMEnumToInt<EAMGroup>(InGroup)].Slots[AMEnumToInt<EAMSlot>(InSlot)]();
	}
};

USTRUCT()
struct FAMAbilitySlotConfig
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
		uint8 SlotNum;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ABILITYMANAGER_API UAMAbilityManagerComponent : public UActorComponent
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere)
		TArray<FAMAbilitySlotConfig> Groups;

	//Map input bindings to particular slot and group.
	UPROPERTY(EditAnywhere)
		FAMAbilityInputContainer InputSetup;

	UPROPERTY(EditAnywhere, Category = "Input Config")
		TArray<FGameplayTag> InputsToBind;

	EAMGroup ActiveGroup;

	TArray<TArray<TSoftClassPtr<UGAAbilityBase>>> AbilityTagsSet;
	TArray<TArray<TWeakObjectPtr<class UGAAbilityBase>>> AbilitySet;
	TArray<TArray<bool>> ValidGroups;
	TMap<TSoftClassPtr<UGAAbilityBase>, FSimpleDelegate> AbilityReadyEvents;

	DECLARE_DELEGATE_TwoParams(FGroupConfirmDelegate, int32, bool)

	FGroupConfirmDelegate OnNextGroupDelegate;
	FGroupConfirmDelegate OnPreviousGroupDelegate;

public:	
	// Sets default values for this component's properties
	UAMAbilityManagerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void InitializeComponent() override;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void BindInputs(UInputComponent* InputComponent, class UAFAbilityComponent* AbilityComponent);
	UGAAbilityBase* GetAbility(EAMGroup InGroup, EAMSlot InSlot);
	void SetAbility(EAMGroup InGroup, EAMSlot InSlot, UGAAbilityBase* InAbility);
	void RemoveAbility(EAMGroup InGroup, EAMSlot InSlot);

	TSoftClassPtr<UGAAbilityBase> GetAbilityTag(EAMGroup InGroup, EAMSlot InSlot);
	void SetAbilityTag(EAMGroup InGroup, EAMSlot InSlot, TSoftClassPtr<UGAAbilityBase> InAbilityTag);
	void RemoveAbilityTag(EAMGroup InGroup, EAMSlot InSlot);

	TArray<FGameplayTag> GetInputTag(EAMGroup InGroup, EAMSlot InSlot);
	void SetInputTag(EAMGroup InGroup, EAMSlot InSlot, TArray<FGameplayTag> InAbilityTag);

	void NativeEquipAbility(TSoftClassPtr<UGAAbilityBase> InAbilityTag, EAMGroup InGroup, EAMSlot InSlot, bool bBindInput = true);
	void NativeRemoveAbility(TSoftClassPtr<UGAAbilityBase> InAbilityTag, EAMGroup InGroup, EAMSlot InSlot);
protected:
	virtual void OnAbilityReady(TSoftClassPtr<UGAAbilityBase> InAbilityTag, const TArray<FGameplayTag>& InAbilityInput,
		EAMGroup InGroup, EAMSlot InSlot) {};
private:
	UFUNCTION()
		void OnAbilityReadyInternal(TSoftClassPtr<UGAAbilityBase> InAbilityTag, TArray<FGameplayTag> InAbilityInput,
			EAMGroup InGroup, EAMSlot InSlot, bool bBindInput);

public:
	UFUNCTION()
		void OnAbilityInputReady(TSoftClassPtr<UGAAbilityBase> InAbilityTag, TArray<FGameplayTag> InAbilityInput,
			EAMGroup InGroup, EAMSlot InSlot);
	
	UFUNCTION(BlueprintCallable)
		virtual void NextGroup();
	UFUNCTION(BlueprintCallable)
		virtual void PreviousGroup();

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerNextGroup(int32 WeaponIndex);
	void ServerNextGroup_Implementation(int32 WeaponIndex);
	bool ServerNextGroup_Validate(int32 WeaponIndex);
	UFUNCTION(Client, Reliable)
		void ClientNextGroup(int32 WeaponIndex, bool bPredictionSuccess);
	void ClientNextGroup_Implementation(int32 WeaponIndex, bool bPredictionSuccess);

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerPreviousGroup(int32 WeaponIndex);
	void ServerPreviousGroup_Implementation(int32 WeaponIndex);
	bool ServerPreviousGroup_Validate(int32 WeaponIndex);
	UFUNCTION(Client, Reliable)
		void ClientPreviousGroup(int32 WeaponIndex, bool bPredictionSuccess);
	void ClientPreviousGroup_Implementation(int32 WeaponIndex, bool bPredictionSuccess);


	virtual void OnNextGroupConfirmed(EAMGroup ValidGroup, bool bPredictionSuccess) {};
	virtual void OnPreviousGroupConfirmed(EAMGroup ValidGroup, bool bPredictionSuccess) {};
	virtual void OnGroupSelectionConfirmed(EAMGroup ValidGroup, bool bPredictionSuccess) {};

	UFUNCTION(BlueprintCallable)
		void SelectGroup(EAMGroup InGroup);
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerSelectGroup(EAMGroup InGroup);
	void ServerSelectGroup_Implementation(EAMGroup InGroup);
	bool ServerSelectGroup_Validate(EAMGroup InGroup);
	UFUNCTION(Client, Reliable)
		void ClientSelectGroup(EAMGroup InGroup, bool bPredictionSuccess);
	void ClientSelectGroup_Implementation(EAMGroup InGroup, bool bPredictionSuccess);


	void AddOnAbilityReadyEvent(const TSoftClassPtr<UGAAbilityBase>& Ability, const FSimpleDelegate& Delegate)
	{
		if (!AbilityReadyEvents.Contains(Ability))
		{
			AbilityReadyEvents.Add(Ability, Delegate);
		}
	}

	void ExecuteAbilityReadyEvent(const TSoftClassPtr<UGAAbilityBase>& Ability)
	{
		if (FSimpleDelegate* Event = AbilityReadyEvents.Find(Ability))
		{
			Event->ExecuteIfBound();
			AbilityReadyEvents.Remove(Ability);
		}
	}
protected:
	class UAFAbilityComponent* GetAbilityComponent();
	void BindOnAbilityReadDelegate(TSoftClassPtr<UGAAbilityBase>, TArray<FGameplayTag> InAbilityInput,
		EAMGroup InGroup, EAMSlot InSlot);
	bool IsServerOrStandalone() const;
	bool IsClientOrStandalone() const;
	bool IsClient() const;
};
