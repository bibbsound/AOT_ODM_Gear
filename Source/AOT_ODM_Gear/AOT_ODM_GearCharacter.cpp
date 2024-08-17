// Copyright Epic Games, Inc. All Rights Reserved.

#include "AOT_ODM_GearCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "ODM_Gear.h"

#include "GameplayAbilities/GameplayAbilityInputs.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilities/PlayerAttributeSet.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AAOT_ODM_GearCharacter::AAOT_ODM_GearCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	AbilitySystemComp = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	PlayerAttributeSet = CreateDefaultSubobject<UPlayerAttributeSet>(TEXT("PlayerAttributeSet"));

	bShouldActivateGrappleAbility = true;

	bCanGrapple = false;

	ODM_Gear_Socket = "ODM_Socket";
}

void AAOT_ODM_GearCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// Setup Gameplay abilities
	if (HasAuthority())
	{
		SetupInitialAbilitiesAndEffects();
	}

	FTransform SpawnSocketTransform = GetMesh()->GetSocketTransform(ODM_Gear_Socket);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ODM_Gear = GetWorld()->SpawnActor<AODM_Gear>(ODM_GearClass, SpawnSocketTransform.GetLocation(), SpawnSocketTransform.GetRotation().Rotator(), SpawnParams);
	if(ODM_Gear)
	{
		FAttachmentTransformRules TransformRules = FAttachmentTransformRules::SnapToTargetIncludingScale;
		ODM_Gear->AttachToComponent(GetMesh(), TransformRules, ODM_Gear_Socket);
	}

}

void AAOT_ODM_GearCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
	if (bShouldActivateGrappleAbility)
	{
		AbilitySystemComp->TryActivateAbilityByClass(GrappleAbilityFindValidTarget);
	}


	if(bIsGrappling)
	{
		if (GrappleTargetIndicators.Num() > 0)
		{
			TArray<AActor*> TargetKeys;
			GrappleTargetIndicators.GetKeys(TargetKeys);

			// If one valid target and one cable has been attached, use the target as an anchor point to allow the player to swing (more air control)
			if(TargetKeys.Num() == 1)
			{
				UE_LOG(LogTemp, Warning, TEXT("1 target"));
				AActor* FirstGrappleTarget = TargetKeys[0];
				BP_AddForce(FirstGrappleTarget);
			}

			// If two valid grapple points, the player will use them as anchor points to launch themselves forwards 
			if(TargetKeys.Num() == 2)
			{
				UE_LOG(LogTemp, Warning, TEXT("2 target"));

				AActor* FirstGrappleTarget = TargetKeys[0];
				AActor* SecondGrappleTarget = TargetKeys[1];

				// Calculate the midpoint of the two targets 
				//FVector Midpoint = (FirstGrappleTarget->GetActorLocation() + SecondGrappleTarget->GetActorLocation()) / 2.0f;

				//// Get direction of midpoint
				//FVector LaunchDirection = (Midpoint - GetActorLocation()).GetSafeNormal();

				//// Calculate velocity of the launch 
				//FVector LaunchVelocity = LaunchDirection * LaunchStrength;

				//LaunchCharacter(LaunchVelocity, true, true);


				// Calculate the midpoint of the two targets 
				FVector Midpoint = (FirstGrappleTarget->GetActorLocation() + SecondGrappleTarget->GetActorLocation()) / 2.0f;

				// Calculate the distance from the player to the midpoint
				float DistanceToMidpoint = FVector::Dist(GetActorLocation(), Midpoint);


				if (bMidpointLaunch)
				{
					if (DistanceToMidpoint > 100.0f)  // Adjust the threshold as needed
					{

						UE_LOG(LogTemp, Warning, TEXT("Midpoint launch"));
						FVector LaunchDirection = (Midpoint - GetActorLocation()).GetSafeNormal();
						FVector LaunchVelocity = LaunchDirection * LaunchStrength;

						// Launch the player towards the midpoint
						LaunchCharacter(LaunchVelocity, true, true);
					}

					else
					{
						bMidpointLaunch = false;
					}

				}
				
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("AFTER Midpoint launch"));

					FVector CameraLocation;
					FRotator CameraRotation;
					GetController()->GetPlayerViewPoint(CameraLocation, CameraRotation);

					FVector LaunchDirection = CameraRotation.Vector();
					FVector LaunchVelocity = LaunchDirection * LaunchStrength * 2.0f;

					// Continue moving in the same direction
					LaunchCharacter(LaunchVelocity, true, true);
					StopGrapple();
				}
			}
		}
	}
}

#pragma region Inputs

void AAOT_ODM_GearCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInputComponent->BindAction(GrappleAction, ETriggerEvent::Started, this, &AAOT_ODM_GearCharacter::StartGrapple);
		EnhancedInputComponent->BindAction(GrappleAction, ETriggerEvent::Completed, this, &AAOT_ODM_GearCharacter::StopGrapple);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAOT_ODM_GearCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAOT_ODM_GearCharacter::Look);

		// Bind gameplay abilities to inputs
		for (const FAbilityInputToInputActionBinding& binding : AbilityInputBindings.Bindings)
		{
			EnhancedInputComponent->BindAction(binding.InputAction, ETriggerEvent::Started, this, &ThisClass::AbilityInputBindingPressedHandler, binding.AbilityInput);
			EnhancedInputComponent->BindAction(binding.InputAction, ETriggerEvent::Completed, this, &ThisClass::AbilityInputBindingReleasedHandler, binding.AbilityInput);
		}
	}

	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AAOT_ODM_GearCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		MoveRight = MovementVector.X;
		//UE_LOG(LogTemp, Warning, TEXT("MoveRight: %f"), MoveRight);

		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		
		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AAOT_ODM_GearCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

#pragma endregion

#pragma region Gameplay Abilities

void AAOT_ODM_GearCharacter::StartGrapple()
{
	if(bIsGrappling)
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);

		UE_LOG(LogTemp, Warning, TEXT("Grappling"));

		//GetCharacterMovement()->GravityScale = 0.2f;

		BP_PlayMontage();
	}
}

void AAOT_ODM_GearCharacter::StopGrapple()
{
	if(bIsGrappling)
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
		bIsGrappling = false;

		//@TODO check which cable is attached and detach that cable
		ODM_Gear->DetattachGrappleCable(ODM_Gear->GetLeftCableComponent());
		ODM_Gear->DetattachGrappleCable(ODM_Gear->GetRightCableComponent());

		bMidpointLaunch = true;

		UE_LOG(LogTemp, Error, TEXT("Grappling FINISHED"));

		//GetCharacterMovement()->GravityScale = 1.0f;
	}
}

void AAOT_ODM_GearCharacter::SetupInitialAbilitiesAndEffects()
{
	if (IsValid(AbilitySystemComp) == false || IsValid(PlayerAttributeSet) == false)
	{
		return;
	}

	if (IsValid(InitialAbilitySet))
	{
		InitiallyGrantedAbilitySpecHandle.Append(InitialAbilitySet->GrantAbilitiesToAbilitySystem(AbilitySystemComp));
	}

	// Apply any initial effects
	if (IsValid(InitialGameplayEffect))
	{
		AbilitySystemComp->ApplyGameplayEffectToSelf(InitialGameplayEffect->GetDefaultObject<UGameplayEffect>(), 0.0f, AbilitySystemComp->MakeEffectContext());
	}

	// Grant the find valid grapple point ability
	if(IsValid(GrappleAbilityFindValidTarget))
	{
		AbilitySystemComp->GiveAbility(FGameplayAbilitySpec(GrappleAbilityFindValidTarget, 1, 0));
	}

	AbilitySystemComp->GetGameplayAttributeValueChangeDelegate(UPlayerAttributeSet::GetHealthAttribute()).AddUObject(this, &ThisClass::OnHealthAttributeChanged);
}

void AAOT_ODM_GearCharacter::OnHealthAttributeChanged(const FOnAttributeChangeData& onAttributeChangeData) const
{
}

void AAOT_ODM_GearCharacter::AbilityInputBindingPressedHandler(EAbilityInput abilityInput)
{
	AbilitySystemComp->AbilityLocalInputPressed(static_cast<uint32>(abilityInput));
}

void AAOT_ODM_GearCharacter::AbilityInputBindingReleasedHandler(EAbilityInput abilityInput)
{
	AbilitySystemComp->AbilityLocalInputReleased(static_cast<uint32>(abilityInput));
}

#pragma endregion