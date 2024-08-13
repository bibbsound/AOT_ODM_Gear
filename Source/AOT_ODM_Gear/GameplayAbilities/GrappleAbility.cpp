// Fill out your copyright notice in the Description page of Project Settings.


#include "AOT_ODM_Gear/GameplayAbilities/GrappleAbility.h"
#include "AOT_ODM_Gear/AOT_ODM_GearCharacter.h"
#include "AOT_ODM_Gear/ODM_Gear.h"
#include "GameFramework/CharacterMovementComponent.h"

void UGrappleAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogTemp, Warning, TEXT("Grapple ability activated"));

    if (AAOT_ODM_GearCharacter* PlayerCharacter = Cast<AAOT_ODM_GearCharacter>(GetAvatarActorFromActorInfo()))
    {
        if(PlayerCharacter->GetbCanGrapple())
        {
            UE_LOG(LogTemp, Warning, TEXT("Valid Grapple Target, Grapple towards target"));

            // valid targets
            if (PlayerCharacter->GrappleTargetIndicators.Num() > 0)
            {
               // // create iterator over targets 
               // auto It = PlayerCharacter->GrappleTargetIndicators.CreateIterator();

               // // get the first target
               // AActor* FirstGrappleTarget = It.Key();

               //// if valid 
               // if (FirstGrappleTarget)
               // {
               //     // Attach the cable to the target 
               //     UE_LOG(LogTemp, Log, TEXT("First Grapple Target: %s"), *FirstGrappleTarget->GetName());
               //     PlayerCharacter->GetODMGearActor()->AttachGrappleCable(PlayerCharacter->GetODMGearActor()->GetRightCableComponent(), FirstGrappleTarget);
               //     PlayerCharacter->GetODMGearActor()->AttachGrappleCable(PlayerCharacter->GetODMGearActor()->GetLeftCableComponent(), FirstGrappleTarget);
               //     //PlayerCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
               //     PlayerCharacter->SetbIsGrappling(true);
               // }

                TArray<AActor*> TargetKeys;
                PlayerCharacter->GrappleTargetIndicators.GetKeys(TargetKeys);


                // two valid grapple targets
                if(TargetKeys.Num() == 2)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Two grapple targets "));

                    AActor* FirstGrapleTarget = TargetKeys[0];
                    AActor* SecondGrappleTarget = TargetKeys[1];
                }

                // 1 valid grapple target 
                else if(TargetKeys.Num() == 1) 
                {
                    UE_LOG(LogTemp, Warning, TEXT("one  grapple target "));

                    AActor* GrapleTarget = TargetKeys[0];

                    FVector GrappleTargetRV = GrapleTarget->GetActorRightVector();
                    FVector GrappleTargetFV = GrapleTarget->GetActorForwardVector();

                    FVector TargetToPlayer = GrapleTarget->GetActorLocation() - PlayerCharacter->GetActorLocation();
                    TargetToPlayer.Normalize();

                    FVector Cross = FVector::CrossProduct(GrappleTargetFV, TargetToPlayer);

                    //float Angle = FVector::DotProduct(GrappleTargetRV, TargetToPlayer);

                    if(Cross.Z > 0)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Player is to the right of target "));
                    }

                    else if(Cross.Z < 0)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Player is to the left of target "));
                    }

                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Player is to infront of target "));
                    }
                }

                // No valid grapple targets 
                else
                {
                    return;
                }










            }
        }
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
