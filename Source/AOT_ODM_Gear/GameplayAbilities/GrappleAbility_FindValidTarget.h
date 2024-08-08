// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GrappleAbility_FindValidTarget.generated.h"


UCLASS()
class AOT_ODM_GEAR_API UGrappleAbility_FindValidTarget : public UGameplayAbility
{
	GENERATED_BODY()
	
private:

	UGrappleAbility_FindValidTarget();

	UPROPERTY(EditDefaultsOnly, Category = "Grapple Ability")
	float MaxGrappleDistance;

	class AAOT_ODM_GearCharacter* PlayerCharacter;

	class APlayerController* PlayerController;

	AActor* PreviousGrappleTarget = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Grapple Ability")
	TSubclassOf<AActor> IndicatorClass;

	AActor* SpawnedIndicatorActor;

	UPROPERTY()
	TArray<AActor*> ValidGrappleTargets;

	UPROPERTY()
	AActor* LineTraceHitActor;

	// Used to find grapple objects directly infront of the player
	UFUNCTION()
	void PerformLineTrace();

	UFUNCTION()
	void PerformSphereTrace();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:

	UFUNCTION(BlueprintImplementableEvent)
	void BP_ValidGrappleTarget();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_InValidGrappleTarget();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_BestGrappleTarget(AActor* Target);
};
