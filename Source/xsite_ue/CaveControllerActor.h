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
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFilePath ConfigurationFilePath = {"Calibration.json"};

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDirectoryPath BlendTextureFolderPath = {""};

    UPROPERTY()
    FVector EyeOrigin;
private:
    void LoadWallScreenConfig();
};
