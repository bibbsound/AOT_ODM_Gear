// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "GameplayAbilities/GameplayAbilityInputs.h"
#include "AOT_ODM_GearCharacter.generated.h"

USTRUCT()
struct FAbilityInputToInputActionBinding
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	class UInputAction* InputAction;

	UPROPERTY(EditDefaultsOnly)
	EAbilityInput AbilityInput;
};

USTRUCT(BlueprintType)
struct FAbilityInputBindings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TArray<FAbilityInputToInputActionBinding> Bindings;
};

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class AODM_Gear;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AAOT_ODM_GearCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

private:

	// in future will be tied to picking up or equipping grapple ability from loadout system
	bool bShouldActivateGrappleAbility;

	// Allows the player to grapple to a grapple point
	bool bCanGrapple;

	AODM_Gear* ODM_Gear;

	UPROPERTY(EditDefaultsOnly, Category = "ODM_Gear")
	TSubclassOf<AODM_Gear> ODM_GearClass;

	FName ODM_Gear_Socket;

public:

	AODM_Gear* GetODMGearActor() const { return ODM_Gear; }

	void SetbCanGrapple(bool NewValue) { bCanGrapple = NewValue; }

	bool GetbCanGrapple() const { return bCanGrapple; }

	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<UGameplayAbility> GrappleAbilityFindValidTarget;

	// Will find valid grapple points via line trace
	UPROPERTY(VisibleAnywhere, Category = "Abilities")
	class UAbilitySystemComponent* AbilitySystemComp{ nullptr };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	class UPlayerAttributeSet* PlayerAttributeSet{ nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	class UGameplayAbilityInputs* InitialAbilitySet;

	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	UDataAsset* DA;

	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<class UGameplayEffect> InitialGameplayEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	FAbilityInputBindings AbilityInputBindings;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComp; }

	void SetupInitialAbilitiesAndEffects();

	void OnHealthAttributeChanged(const FOnAttributeChangeData& onAttributeChangeData) const;

	UPROPERTY(Transient)
	TArray<FGameplayAbilitySpecHandle> InitiallyGrantedAbilitySpecHandle;

	void AbilityInputBindingPressedHandler(EAbilityInput abilityInput);

	void AbilityInputBindingReleasedHandler(EAbilityInput abilityInput);

public:
	AAOT_ODM_GearCharacter();
	

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
			

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

	virtual void Tick(float DeltaTime);

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

