// Fill out your copyright notice in the Description page of Project Settings.


#include "AOT_ODM_Gear/GameplayAbilities/GameplayAbilityInputs.h"
#include "AbilitySystemComponent.h"

TArray<FGameplayAbilitySpecHandle> UGameplayAbilityInputs::GrantAbilitiesToAbilitySystem(UAbilitySystemComponent* AbilitySystemComp) const
{
	check(AbilitySystemComp);

	TArray<FGameplayAbilitySpecHandle> handles;
	handles.Reserve(AbilitySetItems.Num());

	for (const auto& abilitySetItem : AbilitySetItems)
	{
		handles.AddUnique(AbilitySystemComp->GiveAbility(FGameplayAbilitySpec(abilitySetItem.GameplayAbility, 0, static_cast<uint32>(abilitySetItem.InputKey))));
	}

	return handles;
}