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

        // Handle UI indicators
        TMap<AActor*, UWidgetComponent*> NewGrappleTargetIndicators;

        for (AActor* Target : ValidGrappleTargets)
        {
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

            // Test UI indictors - V1
            //FVector ProjectedPoint = CameraLocation + (CameraForwardVector * 50);

            //if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Target->GetRootComponent()))
            //{
            //    FVector ClosestPoint;

            //    PrimitiveComponent->GetClosestPointOnCollision(PlayerCharacter->GetActorLocation(), ClosestPoint);

            //    DrawDebugSphere(GetWorld(), ClosestPoint, 20.0f, 12, FColor::Green, false, 1.0f);

            //    //DrawDebugLine(GetWorld(), CameraLocation, ProjectedPoint, FColor::Red, false, 1.0f, 0.0f, 1.0f);
            //}

            // Test UI indictors - V2
            //FVector ProjectedPoint = CameraLocation + (CameraForwardVector * 100.0f); // 1000 units in front of the camera

            //if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Target->GetRootComponent()))
            //{
            //    FVector ClosestPoint;

            //    
            //    PrimitiveComponent->GetClosestPointOnCollision(PlayerCharacter->smc->GetComponentLocation(), ClosestPoint);

            //    //ClosestPoint.Z *= -1.0f;
            //     
            //    DrawDebugSphere(GetWorld(), ClosestPoint, 20.0f, 12, FColor::Green, false, 1.0f);

            //    UE_LOG(LogTemp, Warning, TEXT("X: %f, Y: %f, Z: %f"), ClosestPoint.X, ClosestPoint.Y, ClosestPoint.Z);
            //    //DrawDebugLine(GetWorld(), CameraLocation, ClosestPoint, FColor::Red, false, 1.0f, 0.0f, 1.0f);
            //}


            // Test UI indictors - V3
     
            // Line trace in forward direction (same distance to target)
            
            // use end point of line trace to find closest point on target actor

            FVector CameraTraceEnd = CameraLocation + (CameraForwardVector * MaxGrappleDistance);

            FHitResult CameraTraceHitResult;
            FCollisionQueryParams CameraTraceQueryParams;

            bool bHit = GetWorld()->LineTraceSingleByChannel(CameraTraceHitResult, CameraLocation, CameraTraceEnd, ECC_Visibility, CameraTraceQueryParams);
            DrawDebugLine(GetWorld(), CameraLocation, CameraTraceEnd, FColor::Red, false, 1.0f, 0.0f, 1.0f);

            // If the camera trace hits an object
            if(bHit)
            {
                // If the hit object is a grapple target, 
                if(CameraTraceHitResult.GetActor() == Target)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Hit Grapple Actor"));
                    DrawDebugSphere(GetWorld(), CameraTraceHitResult.ImpactPoint, 20.0f, 12, FColor::Green, false, 1.0f);
                }

                // If the hit object is NOT a grapple target, then use the hit location to get the closest point to the grapple target
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Hit Actor"));
                    if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Target->GetRootComponent()))
                    {
                        FVector ClosestPoint;

                        PrimitiveComponent->GetClosestPointOnCollision(CameraTraceHitResult.ImpactPoint, ClosestPoint);

                        DrawDebugSphere(GetWorld(), ClosestPoint, 20.0f, 12, FColor::Green, false, 1.0f);

                    }
                }

                
            }

            // If the camera trace did not hit anything then use endpoint of trace
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("NO Hit "));

				if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Target->GetRootComponent()))
				{
					FVector ClosestPoint;

					PrimitiveComponent->GetClosestPointOnCollision(CameraTraceEnd, ClosestPoint);

					DrawDebugSphere(GetWorld(), ClosestPoint, 20.0f, 12, FColor::Green, false, 1.0f);

					
				}

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



