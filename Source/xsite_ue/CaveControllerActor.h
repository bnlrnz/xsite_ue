// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VRPNController.h"
#include "CaveCalibrationParser.h"
#include "CaveControllerActor.generated.h"

UCLASS()
class  ACaveControllerActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACaveControllerActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFilePath ConfigurationFile = {"Calibration.json"};

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString VRPN_DEVICE = "DTrack";
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString VRPN_HOST = "139.20.18.74";

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString VRPN_DEVICE_WII = "WiiMote0";

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString VRPN_HOST_WII = "127.0.0.1";

    VRPNController* GetVRPNController();
    VRPNController* GetVRPNWIIController();

    UPROPERTY()
    FVector EyeOrigin;
private:
    VRPNController* vrpnController = nullptr;
    VRPNController* vrpnWiiController = nullptr;
	
    void LoadWallScreenConfig();
};
