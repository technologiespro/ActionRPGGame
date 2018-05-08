// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ARHUD.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONRPGGAME_API AARHUD : public AHUD
{
	GENERATED_BODY()
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|UI")
		class UARUIInventoryComponent* UIInventoryComponent;
	
public:
	AARHUD(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	inline UARUIInventoryComponent* GetUIInventory()
	{
		return UIInventoryComponent;
	}


	void ShowHideInventory();
};
