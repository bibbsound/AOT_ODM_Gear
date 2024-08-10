// Fill out your copyright notice in the Description page of Project Settings.


#include "AOT_ODM_Gear/GameplayAbilities/GrappleAbility_FindValidTarget.h"
#include "DrawDebugHelpers.h"
#include "AOT_ODM_Gear/AOT_ODM_GearCharacter.h"
//#include "Blueprint/UserWidget.h"
#include "Components/WidgetComponent.h"
#include "Kismet/KismetSystemLibrary.h"
//#include "GameFramework/Character.h"

UGrappleAbility_FindValidTarget::UGrappleAbility_FindValidTarget()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    MaxGrappleDistance = 5000.0f;

    MaxGrappleAngle = 0.5f;
}

// not used
void UGrappleAbility_FindValidTarget::PerformLineTrace()
{
    if (PlayerCharacter)
    {
        if (PlayerController)
        {
            FVector CameraLocation;
            FRotator CameraRotation;
            PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

            FVector TraceEnd = CameraLocation + (CameraRotation.Vector() * MaxGrappleDistance);

            FHitResult HitResult;
            FCollisionQueryParams QueryParams;
            QueryParams.AddIgnoredActor(PlayerCharacter);

            bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, TraceEnd, ECC_Visibility, QueryParams);

            if (bHit && HitResult.GetActor() && HitResult.GetActor()->ActorHasTag(FName("GrappleTarget")))
            {
                DrawDebugLine(GetWorld(), CameraLocation, HitResult.Location, FColor::Red, false, 1.0f, 0, 1.0f);

                PlayerCharacter->SetbCanGrapple(true);

                LineTraceHitActor = HitResult.GetActor();

                // Add this grapple target to array of valid targets
                //ValidGrappleTargets.Add(HitResult.GetActor());

                BP_ValidGrappleTarget(); // Change UI indicator in bp to valid 
            }

            else
            {
                DrawDebugLine(GetWorld(), CameraLocation, TraceEnd, FColor::Green, false, 1.0f, 0, 1.0f);

                PlayerCharacter->SetbCanGrapple(false);

                LineTraceHitActor = nullptr;

                BP_InValidGrappleTarget(); // Change UI indicator in bp to invalid 
            }
        }
    }
}

void UGrappleAbility_FindValidTarget::PerformSphereTrace()
{
    if (PlayerCharacter && PlayerController)
    {
        /* Spawn a sphere trace to find all actors in range */
        FVector CameraLocation;
        FRotator CameraRotation;
        PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

        FVector TraceEnd = CameraLocation + (CameraRotation.Vector() * MaxGrappleDistance);
        float SphereRadius = MaxGrappleDistance;

        TArray<FHitResult> HitResults;
        //bool bSphereHit = GetWorld()->SweepMultiByChannel(HitResults, CameraLocation, TraceEnd, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(SphereRadius));

        //DrawDebugSphere(GetWorld(), CameraLocation, SphereRadius, 12, FColor::Orange, false, 2.0f);

        //bool bLineTraceHit = GetWorld()->LineTraceMultiByChannel(HitResults, CameraLocation, TraceEnd, ECC_Visibility);

        // Draw a debug line to visualize the trace
        //DrawDebugSphere(GetWorld(), CameraLocation, 100.0f, 12, FColor::Blue, false, 1.0f);
        //DrawDebugSphere(GetWorld(), TraceEnd, 100.0f, 12, FColor::Red, false, 1.0f);

        //FCollisionShape MySphere = FCollisionShape::MakeSphere(SphereRadius); // 5M Radius
        //bool b1 = GetWorld()->SweepMultiByChannel(HitResults, CameraLocation, TraceEnd, FQuat::Identity, ECC_Visibility, MySphere);
        //DrawDebugSphere(GetWorld(), CameraLocation, SphereRadius, 12, FColor::Orange, false, 2.0f);

        
       

        /* Find valid grapple targets */

       /* if(b1)
        {
            TArray<AActor*> AllGrappleTargets;
            for (const FHitResult& Hit : HitResults)
            {
                AActor* HitActor = Hit.GetActor();
                if (HitActor && HitActor->ActorHasTag(FName("GrappleTarget")))
                {
                    AllGrappleTargets.Add(HitActor);

                    UE_LOG(LogTemp, Warning, TEXT("Grapple Actor: %s"), *HitActor->GetFullName());
                }
            }
        }*/
       

        /* Find the best two grapple points (This is based on distance and angle from player)*/
        //TArray<AActor*> ValidGrappleTargets;

        //FVector PlayerForwardVector = PlayerCharacter->GetActorForwardVector();

        //for (AActor* Target : AllGrappleTargets)
        //{
        //    FVector DirectionToTarget = (Target->GetActorLocation() - CameraLocation).GetSafeNormal();
        //    float DotProduct = FVector::DotProduct(PlayerForwardVector, DirectionToTarget);

        //    if (DotProduct > MaxGrappleAngle)
        //    {
        //        ValidGrappleTargets.Add(Target);
        //    }
        //}

        ///* Sort grapple targets by distance to the camera, this allows for a max of two grapple targets (similar to ODM) */
        //ValidGrappleTargets.Sort([&](const AActor& A, const AActor& B)
        //    {
        //        float DistanceA = FVector::Dist(CameraLocation, A.GetActorLocation());
        //        float DistanceB = FVector::Dist(CameraLocation, B.GetActorLocation());
        //        return DistanceA < DistanceB;
        //    });

        //// Allow for only 2 grapple points
        //if (ValidGrappleTargets.Num() > 2)
        //{
        //    ValidGrappleTargets.SetNum(2);
        //}

        //// Handle indicators, temporary until UI indicators are used
        //TMap<AActor*, UWidgetComponent*> NewGrappleTargetIndicators;

        //for (AActor* Target : ValidGrappleTargets)
        //{
        //    if (GrappleTargetIndicators.Contains(Target))
        //    {
        //        // If the indicator already exists, just keep it
        //        NewGrappleTargetIndicators.Add(Target, GrappleTargetIndicators[Target]);
        //        GrappleTargetIndicators.Remove(Target);
        //    }
        //    else
        //    {
        //        // Create a new widget component for the grapple point
        //        UWidgetComponent* WidgetComp = NewObject<UWidgetComponent>(Target);

        //        if (WidgetComp && GrapplePointWidget)
        //        {
        //            WidgetComp->SetupAttachment(Target->GetRootComponent()); // Attach to the target actor
        //            WidgetComp->SetWidgetClass(GrapplePointWidget);
        //            WidgetComp->SetRelativeLocation(FVector::ZeroVector);
        //            WidgetComp->SetWidgetSpace(EWidgetSpace::Screen); // Use screen space for 2D UI
        //            WidgetComp->RegisterComponent();

        //            NewGrappleTargetIndicators.Add(Target, WidgetComp);
        //        }
        //    }
        //}

        //// Destroy indicators for targets that are no longer valid
        //for (auto& IndicatorPair : GrappleTargetIndicators)
        //{
        //    if (IndicatorPair.Value)
        //    {
        //        IndicatorPair.Value->DestroyComponent();
        //    }
        //}

        //// Update the map with the current valid targets and their indicators
        //GrappleTargetIndicators = NewGrappleTargetIndicators;
    }
}

void UGrappleAbility_FindValidTarget::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
   // UE_LOG(LogTemp, Warning, TEXT("Firing ability activated"));

    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        // Validate pointers to player and controller 
        PlayerCharacter = Cast<AAOT_ODM_GearCharacter>(GetAvatarActorFromActorInfo());

        if(PlayerCharacter)
        {
            PlayerController = Cast<APlayerController>(PlayerCharacter->GetController());
        }

        //UE_LOG(LogTemp, Warning, TEXT("Firing ability valid"));

        LineTraceHitActor = nullptr;

        //PerformLineTrace();

        PerformSphereTrace();

        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    }
}


