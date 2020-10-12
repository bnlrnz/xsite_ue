// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VRPNController.h"
#include "VRPNControllerActor.generated.h"

UCLASS()
class XSITE_UE_API AVRPNControllerActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVRPNControllerActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString VRPN_Device_Name = "DTrack";
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString VRPN_Host_IP = "139.20.18.74";
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    VRPNController* GetController();
private:
    VRPNController* vrpnController = nullptr;
};
