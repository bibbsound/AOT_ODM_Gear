// Fill out your copyright notice in the Description page of Project Settings.


#include "AOT_ODM_Gear/GameplayAbilities/GrappleAbility.h"
#include "AOT_ODM_Gear/AOT_ODM_GearCharacter.h"
#include "AOT_ODM_Gear/ODM_Gear.h"

void UGrappleAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogTemp, Warning, TEXT("Grapple ability activated"));

    if (AAOT_ODM_GearCharacter* PlayerCharacter = Cast<AAOT_ODM_GearCharacter>(GetAvatarActorFromActorInfo()))
    {
        if(PlayerCharacter->GetbCanGrapple())
        {
            UE_LOG(LogTemp, Warning, TEXT("Valid Grapple Target, Grapple towards target"));

            if (PlayerCharacter->GrappleTargetIndicators.Num() > 0)
            {
                // Get an iterator to the first element in the map
                auto It = PlayerCharacter->GrappleTargetIndicators.CreateIterator();

                // Access the first grapple target
                AActor* FirstGrappleTarget = It.Key();

                // Now you can work with FirstGrappleTarget as needed
                if (FirstGrappleTarget)
                {
                    UE_LOG(LogTemp, Log, TEXT("First Grapple Target: %s"), *FirstGrappleTarget->GetName());
                    PlayerCharacter->GetODMGearActor()->AttachGrappleCable(PlayerCharacter->GetODMGearActor()->GetRightCableComponent(), FirstGrappleTarget);
                }

            }
        }
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
