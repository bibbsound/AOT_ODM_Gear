// Fill out your copyright notice in the Description page of Project Settings.


#include "AOT_ODM_Gear/GameplayAbilities/GrappleAbility_FindValidTarget.h"
#include "DrawDebugHelpers.h"
#include "AOT_ODM_Gear/AOT_ODM_GearCharacter.h"
//#include "GameFramework/Character.h"

UGrappleAbility_FindValidTarget::UGrappleAbility_FindValidTarget()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

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
        FVector CameraLocation;
        FRotator CameraRotation;
        PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

        FVector TraceEnd = CameraLocation + (CameraRotation.Vector() * MaxGrappleDistance);
        float SphereRadius = MaxGrappleDistance;

        TArray<FHitResult> HitResults;
        bool bSphereHit = GetWorld()->SweepMultiByChannel(HitResults, CameraLocation, TraceEnd, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(SphereRadius));

        DrawDebugSphere(GetWorld(), CameraLocation, SphereRadius, 12, FColor::Orange, false, 2.0f);

        // Find valid grapple targets
        TArray<AActor*> AllGrappleTargets;
        for (const FHitResult& Hit : HitResults)
        {
            AActor* HitActor = Hit.GetActor();
            if (HitActor && HitActor->ActorHasTag(FName("GrappleTarget")))
            {
                AllGrappleTargets.Add(HitActor);
            }
        }

        // Find the best grapple targets
        TArray<AActor*> ValidGrappleTargets;

        FVector PlayerForwardVector = PlayerCharacter->GetActorForwardVector();

        for (AActor* Target : AllGrappleTargets)
        {
            FVector DirectionToTarget = (Target->GetActorLocation() - CameraLocation).GetSafeNormal();
            float DotProduct = FVector::DotProduct(PlayerForwardVector, DirectionToTarget);

            if (DotProduct > 0.5f) // Example threshold
            {
                ValidGrappleTargets.Add(Target);
            }
        }

        // Sort targets by distance to the camera
        ValidGrappleTargets.Sort([&](const AActor& A, const AActor& B)
            {
                float DistanceA = FVector::Dist(CameraLocation, A.GetActorLocation());
                float DistanceB = FVector::Dist(CameraLocation, B.GetActorLocation());
                return DistanceA < DistanceB;
            });

        // Limit to the top 2 targets
        if (ValidGrappleTargets.Num() > 2)
        {
            ValidGrappleTargets.SetNum(2); // Keep only the top 2 targets
        }

        // Destroy old indicator actors
        for (AActor* Indicator : SpawnedIndicatorActors)
        {
            if (Indicator)
            {
                Indicator->Destroy();
            }
        }
        SpawnedIndicatorActors.Empty(); // Clear the array

        // Spawn new indicator actors for the best grapple targets
        for (AActor* Target : ValidGrappleTargets)
        {
            if (Target)
            {
                FVector SpawnLocation = Target->GetActorLocation();
                FRotator SpawnRotation = FRotator::ZeroRotator;

                FActorSpawnParameters SpawnParams;
                SpawnParams.Owner = PlayerCharacter;

                // Replace AYourIndicatorActorClass with the actual class of the actor you want to spawn
                AActor* NewIndicator = GetWorld()->SpawnActor<AActor>(IndicatorClass, SpawnLocation, SpawnRotation, SpawnParams);
                if (NewIndicator)
                {
                    NewIndicator->SetActorScale3D(FVector(5.0f));
                    SpawnedIndicatorActors.Add(NewIndicator); // Keep track of spawned indicators
                }

                BP_BestGrappleTarget(Target);
            }
        }

        // Update previous grapple targets
        PreviousGrappleTargets = ValidGrappleTargets;
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


