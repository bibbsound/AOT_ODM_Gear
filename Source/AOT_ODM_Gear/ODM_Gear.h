// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ODM_Gear.generated.h"

class UCableComponent;

UCLASS()
class AOT_ODM_GEAR_API AODM_Gear : public AActor
{
	GENERATED_BODY()
	
private:
	AODM_Gear();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	// Mesh for odm gear
	UPROPERTY(EditDefaultsOnly, Category = "Mesh")
	USkeletalMeshComponent* SKMesh;

	// Where the left cable will spawn 
	FName LeftCableSocket;

	// Where the Right cable will spawn 
	FName RightCableSocket;

	UPROPERTY(EditDefaultsOnly, Category = "Cable")
	UCableComponent* RightCableComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Cable")
	UCableComponent* LeftCableComponent;

public:
	
	UFUNCTION(BlueprintCallable)
	UCableComponent* GetRightCableComponent() const { return RightCableComponent; }

	UFUNCTION(BlueprintCallable)
	UCableComponent* GetLeftCableComponent() const { return LeftCableComponent; }

	void AttachGrappleCable(UCableComponent* Cable, AActor* AttachedActor);

	void DetattachGrappleCable(UCableComponent* Cable);

	bool bRightCableAttached = false;

};
