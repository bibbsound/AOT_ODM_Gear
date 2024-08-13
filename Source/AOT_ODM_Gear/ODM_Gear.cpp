// Fill out your copyright notice in the Description page of Project Settings.


#include "ODM_Gear.h"
#include "CableComponent.h"

// Sets default values
AODM_Gear::AODM_Gear()
{
	//PrimaryActorTick.bCanEverTick = true;

	SKMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ODM Gear Mesh"));

	LeftCableSocket = "LeftCableSocket";

	RightCableSocket = "RightCableSocket";

	RightCableComponent = CreateDefaultSubobject<UCableComponent>(TEXT("Cable Right"));
	RightCableComponent->SetupAttachment(SKMesh, RightCableSocket);

	LeftCableComponent = CreateDefaultSubobject<UCableComponent>(TEXT("Cable Left"));
	LeftCableComponent->SetupAttachment(SKMesh, LeftCableSocket);

	
}

void AODM_Gear::BeginPlay()
{
	Super::BeginPlay();
}

void AODM_Gear::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AODM_Gear::AttachGrappleCable(UCableComponent* Cable, AActor* AttachedActor)
{
	Cable->SetAttachEndTo(AttachedActor, NAME_None, FName());

	//RightCableComponent->SetAttachEndTo(AttachedActor, NAME_None, FName());
	//LeftCableComponent->SetAttachEndTo(AttachedActor, NAME_None, FName());
}

void AODM_Gear::DetattachGrappleCable(UCableComponent* Cable)
{
	Cable->SetAttachEndTo(nullptr, NAME_None, FName());
	UE_LOG(LogTemp, Warning, TEXT("Detach cable"));
}

