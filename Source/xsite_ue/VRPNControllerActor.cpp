// Fill out your copyright notice in the Description page of Project Settings.


#include "VRPNControllerActor.h"


// Sets default values
AVRPNControllerActor::AVRPNControllerActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// this is only needed on server
    bReplicates = false;
}

// Called when the game starts or when spawned
void AVRPNControllerActor::BeginPlay()
{
	Super::BeginPlay();

	if (GetWorld()->IsServer() && vrpnController == nullptr) {
		vrpnController = MakeShareable(new VRPNController());
		vrpnController->Init(VRPN_Device_Name, VRPN_Host_IP, vrpn_DEFAULT_LISTEN_PORT_NO);
	}
}

// Called every frame
void AVRPNControllerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetWorld()->IsServer() && vrpnController != nullptr)
        vrpnController->Poll();
}

TSharedPtr<VRPNController> AVRPNControllerActor::GetController()
{
	return vrpnController;
}


void AVRPNControllerActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	if (vrpnController != nullptr)
	{
		vrpnController.Reset();
	}
}