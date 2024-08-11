// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GrappleAbility_FindValidTarget.generated.h"

class UWidgetComponent;

UCLASS()
class AOT_ODM_GEAR_API UGrappleAbility_FindValidTarget : public UGameplayAbility
{
	GENERATED_BODY()
	
private:

	UGrappleAbility_FindValidTarget();

	UPROPERTY(EditDefaultsOnly, Category = "Grapple Ability")
	float MaxGrappleDistance;

	// How close does the players forward direction have to match the grapple target 
	// (0 - 1, where 1 is close and 0 is perpendicular)
	UPROPERTY(EditDefaultsOnly, Category = "Grapple Ability")
	float MaxGrappleAngle;

	class AAOT_ODM_GearCharacter* PlayerCharacter;

	class APlayerController* PlayerController;

	// Stores the previous grapple targets
	TArray<AActor*> PreviousGrappleTargets;

	// Keeps track of each grapple target and its grapple indicator
	TMap<AActor*, UWidgetComponent*> GrappleTargetIndicators;

	// The widget to be spawned on screen, when there is a valid grapple point
	UPROPERTY(EditDefaultsOnly, Category = "Grapple Ability")
	TSubclassOf<UUserWidget> GrapplePointWidget;

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
