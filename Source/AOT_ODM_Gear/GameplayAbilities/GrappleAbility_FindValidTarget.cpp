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

                BP_ValidGrappleTarget(); // Change UI indicator in bp to valid 
            }

            else
            {
                DrawDebugLine(GetWorld(), CameraLocation, TraceEnd, FColor::Green, false, 1.0f, 0, 1.0f);

                PlayerCharacter->SetbCanGrapple(false);

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

        TArray<FHitResult> HitResults;
        float SphereRadius = MaxGrappleDistance;

        // Ensures that the sphere trace covers the same range as the line trace 
        bool bSphereHit = GetWorld()->SweepMultiByChannel(HitResults, CameraLocation, TraceEnd, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(SphereRadius));

        DrawDebugSphere(GetWorld(), CameraLocation, SphereRadius, 12, FColor::Orange, false, 2.0f);

        // Find a valid grapple target
        TArray<AActor*> ValidGrappleTargets;

        for (const FHitResult& Hit : HitResults) // loop over sphere trace hits
        {
            AActor* HitActor = Hit.GetActor();
            if (HitActor && HitActor->ActorHasTag(FName("GrappleTarget"))) //Check tag to ensure actor is valid grapple target
            {
                ValidGrappleTargets.Add(HitActor);
            }
        }

        
        

        
        
        // Find the best grapple target
        AActor* BestGrappleTarget = nullptr;

        // The best grapple target is determined by distance and the angle from the player to it
        // Ensures that if a grapple target is close to the player but offscreen they will choose a grapple target on screen
        float BestDotProduct = -1.0f;
        FVector PlayerForwardVector = PlayerCharacter->GetActorForwardVector();

        for (AActor* Target : ValidGrappleTargets)
        {
            // Calculate how close a grapple target is aligned with teh players forward direction (Line of sight)
            FVector DirectionToTarget = (Target->GetActorLocation() - CameraLocation).GetSafeNormal();
            float DotProduct = FVector::DotProduct(PlayerForwardVector, DirectionToTarget);

            // Check if the dot product is better and within a valid angle (1 is direcly in front, 0 is perpendicular)
            if (DotProduct > BestDotProduct && DotProduct > 0.5f) // Example threshold
            {
                BestDotProduct = DotProduct;
                BestGrappleTarget = Target;
            }
        }

        // Check if the new target is different from the previous one
        if (BestGrappleTarget != PreviousGrappleTarget)
        {
            // destroy old indicator actor
            if (SpawnedIndicatorActor)
            {
                SpawnedIndicatorActor->Destroy();
                SpawnedIndicatorActor = nullptr;
            }

            // Spawn a new actor at the new best grapple target's location (Place holder until UI is implenmented)
            if (BestGrappleTarget)
            {
                FVector SpawnLocation = BestGrappleTarget->GetActorLocation();
                FRotator SpawnRotation = FRotator::ZeroRotator;
                
                FActorSpawnParameters SpawnParams;
                SpawnParams.Owner = PlayerCharacter;

                // Replace AYourIndicatorActorClass with the actual class of the actor you want to spawn
                SpawnedIndicatorActor = GetWorld()->SpawnActor<AActor>(IndicatorClass, SpawnLocation, SpawnRotation, SpawnParams);
                if(SpawnedIndicatorActor)
                {
                    SpawnedIndicatorActor->SetActorScale3D(FVector(5.0f));
                }

                BP_BestGrappleTarget(BestGrappleTarget);
            }

            // Update previous grapple target to the current target
            PreviousGrappleTarget = BestGrappleTarget;
        }


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

        PerformLineTrace();

        PerformSphereTrace();

        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    }
}


