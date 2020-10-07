// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VRPNController.h"
#include "VPRNControllerActor.generated.h"

UCLASS()
class XSITE_UE_API AVPRNControllerActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVPRNControllerActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString VRPN_Device_Name = "DTrack";
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString VRPN_Host_IP = "139.20.18.74";
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    virtual void BeginDestroy() override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	VRPNController* GetController();
private:
	VRPNController* vrpnController = nullptr;
};
