// Fill out your copyright notice in the Description page of Project Settings.


#include "VPRNControllerActor.h"


// Sets default values
AVPRNControllerActor::AVPRNControllerActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// this is only needed on server
	SetReplicates(false);

	vrpnController = UVRPNController::Create(VRPN_Device_Name, VRPN_Host_IP, vrpn_DEFAULT_LISTEN_PORT_NO);
}

// Called when the game starts or when spawned
void AVPRNControllerActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AVPRNControllerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetWorld()->IsServer())
        vrpnController->Poll();
}

UVRPNController* AVPRNControllerActor::GetController()
{
	return vrpnController;
}
