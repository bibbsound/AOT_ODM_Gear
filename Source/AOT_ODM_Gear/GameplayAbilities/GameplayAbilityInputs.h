// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayAbilitySpec.h"
#include "GameplayAbilityInputs.generated.h"

class UAbilitySystemComponent;

UENUM(BlueprintType)
enum class EAbilityInput : uint8
{
	None = 0,
	Jump = 1,
	PrimaryAttack = 2,
	SecondaryAttack = 3,
};

USTRUCT()
struct FAbilitySetItem
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TSubclassOf<class UGameplayAbility> GameplayAbility;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	EAbilityInput InputKey;
};

UCLASS(Blueprintable)
class AOT_ODM_GEAR_API UGameplayAbilityInputs : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TArray<FAbilitySetItem> AbilitySetItems;

	TArray<FGameplayAbilitySpecHandle> GrantAbilitiesToAbilitySystem(UAbilitySystemComponent* AbilitySystemComp) const;
	
};
