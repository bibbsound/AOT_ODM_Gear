// Fill out your copyright notice in the Description page of Project Settings.


#include "AOT_ODM_Gear/GameplayAbilities/GrappleAbility_FindValidTarget.h"
#include "DrawDebugHelpers.h"
#include "AOT_ODM_Gear/AOT_ODM_GearCharacter.h"
#include "Components/WidgetComponent.h"
#include "Kismet/KismetSystemLibrary.h"

#include "AOT_ODM_Gear/ODM_Gear.h"


UGrappleAbility_FindValidTarget::UGrappleAbility_FindValidTarget()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    MaxGrappleDistance = 5000.0f;

    MaxGrappleAngle = 0.5f;
}

void UGrappleAbility_FindValidTarget::PerformSphereTrace()
{
    if (PlayerCharacter && PlayerController && !PlayerCharacter->GetbIsGrappling())
    {
        /* Spawn a sphere trace to find all actors in range */
        FVector CameraLocation;
        FRotator CameraRotation;
        PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

        FVector CameraForwardVector = CameraRotation.Vector();

        FVector TraceEnd = CameraLocation + (CameraForwardVector * MaxGrappleDistance);
        float SphereRadius = MaxGrappleDistance;

        TArray<FHitResult> HitResults;
        bool bSphereHit = GetWorld()->SweepMultiByChannel(HitResults, CameraLocation, CameraLocation, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(SphereRadius));

        //DrawDebugSphere(GetWorld(), CameraLocation, SphereRadius, 12, FColor::Orange, false, 2.0f);
        
        /* Find valid grapple targets */
		TArray<AActor*> AllGrappleTargets;

        // Loop over all actors hit in the trace
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();

            // Check the hit actors tag
			if (HitActor && HitActor->ActorHasTag(FName("GrappleTarget")))
			{
                // Add grapple targets to array
				AllGrappleTargets.Add(HitActor);

                FVector a = Hit.ImpactPoint;
			}
		}

        /* Find the best two grapple points (This is based on distance and angle from player) */
        TArray<AActor*> ValidGrappleTargets;

        for (AActor* Target : AllGrappleTargets)
        {
            FVector DirectionToTarget = (Target->GetActorLocation() - CameraLocation).GetSafeNormal();
            float DotProduct = FVector::DotProduct(PlayerCharacter->GetActorForwardVector(), DirectionToTarget);

            if (DotProduct > MaxGrappleAngle)
            {
                ValidGrappleTargets.Add(Target);
            }
        }
        
        /* Sort grapple targets by distance to the camera, this allows for a max of two grapple targets (similar to ODM) */
        ValidGrappleTargets.Sort([&](const AActor& A, const AActor& B)
            {
                float DistanceA = FVector::Dist(CameraLocation, A.GetActorLocation());
                float DistanceB = FVector::Dist(CameraLocation, B.GetActorLocation());
                return DistanceA < DistanceB;
            });

        // Allow for only 2 grapple points
        if (ValidGrappleTargets.Num() > 2)
        {
            ValidGrappleTargets.SetNum(2);
        }

        /* Spawn UI indicator on valid grapple target */
        TMap<AActor*, UWidgetComponent*> NewGrappleTargetIndicators;

        for (AActor* Target : ValidGrappleTargets)
        {
            SpawnUIIndicator(Target, NewGrappleTargetIndicators);

            // Test UI indicators
            FVector CameraTraceEnd = CameraLocation + (CameraForwardVector * MaxGrappleDistance);

            FHitResult CameraTraceHitResult;
            FCollisionQueryParams CameraTraceQueryParams;

            bool bHit = GetWorld()->LineTraceSingleByChannel(CameraTraceHitResult, CameraLocation, CameraTraceEnd, ECC_Visibility, CameraTraceQueryParams);
            //DrawDebugLine(GetWorld(), CameraLocation, CameraTraceEnd, FColor::Red, false, 1.0f, 0.0f, 1.0f);

            // If the camera trace hits an object
            if(bHit)
            {
                // If the hit object is a grapple target, grapple point will be on the target 
                if(CameraTraceHitResult.GetActor() == Target)
                {
                    //UE_LOG(LogTemp, Warning, TEXT("Hit Grapple Actor"));
                    DrawDebugSphere(GetWorld(), CameraTraceHitResult.ImpactPoint, 20.0f, 12, FColor::Green, false, 1.0f);
                }

                // If the hit object is NOT a grapple target, then use the hit location to get the closest point to the grapple target and this will be be grapple point
                else
                {
                    //UE_LOG(LogTemp, Warning, TEXT("Hit Actor"));
                
                    GetClosestPointOnActorCollision(Target, CameraTraceHitResult.ImpactPoint);
                }
            }

            // If the camera trace did not hit anything then use endpoint of trace to get the closest point to the grapple target and this will be be grapple point
            else
            {
                //UE_LOG(LogTemp, Warning, TEXT("NO Hit "));

                GetClosestPointOnActorCollision(Target, CameraTraceEnd);
            }

           

























            
            
        }

        // Destroy UI indicators for targets that are no longer valid
        for (auto& IndicatorPair : PlayerCharacter->GrappleTargetIndicators)
        {
            if (IndicatorPair.Value)
            {
                IndicatorPair.Value->DestroyComponent();
            }
        }

        // Update the map with the current valid targets and their indicators
        PlayerCharacter->GrappleTargetIndicators = NewGrappleTargetIndicators;

        // Allow the player to grapple if there is at least 1 valid grapple target 
        PlayerCharacter->GrappleTargetIndicators.Num() > 0 ? PlayerCharacter->SetbCanGrapple(true) : PlayerCharacter->SetbCanGrapple(false);
    }
}

void UGrappleAbility_FindValidTarget::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        // Validate pointers to player and controller 
        PlayerCharacter = Cast<AAOT_ODM_GearCharacter>(GetAvatarActorFromActorInfo());

        if(PlayerCharacter)
        {
            PlayerController = Cast<APlayerController>(PlayerCharacter->GetController());
        }

        PerformSphereTrace();

        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    }
}

FVector UGrappleAbility_FindValidTarget::GetClosestPointOnActorCollision(AActor* Target, FVector StartPoint)
{
    if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Target->GetRootComponent()))
    {
        FVector ClosestPoint;

        PrimitiveComponent->GetClosestPointOnCollision(StartPoint, ClosestPoint);

        DrawDebugSphere(GetWorld(), ClosestPoint, 20.0f, 12, FColor::Green, false, 1.0f);

        return ClosestPoint;
    }

    return StartPoint;
}

void UGrappleAbility_FindValidTarget::SpawnUIIndicator(AActor* Target, TMap<AActor*, UWidgetComponent*>& NewGrappleTargetIndicators)
{
    /* Spawn UI indicator on valid grapple target */
    if (PlayerCharacter->GrappleTargetIndicators.Contains(Target))
    {
        // If the indicator already exists, just keep it
        NewGrappleTargetIndicators.Add(Target, PlayerCharacter->GrappleTargetIndicators[Target]);
        PlayerCharacter->GrappleTargetIndicators.Remove(Target);
    }

    else
    {
        // Create a new widget component for the grapple point
        UWidgetComponent* WidgetComp = NewObject<UWidgetComponent>(Target);

        if (WidgetComp && GrapplePointWidget)
        {
            WidgetComp->SetupAttachment(Target->GetRootComponent()); // Attach to the target actor
            WidgetComp->SetWidgetClass(GrapplePointWidget);
            WidgetComp->SetRelativeLocation(FVector::ZeroVector);
            WidgetComp->SetWidgetSpace(EWidgetSpace::Screen); // Use screen space for 2D UI
            WidgetComp->RegisterComponent();

            NewGrappleTargetIndicators.Add(Target, WidgetComp);
        }
    }
}



