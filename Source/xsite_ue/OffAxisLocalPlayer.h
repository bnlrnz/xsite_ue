// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LocalPlayer.h"
#include "MultiViewportCameraActor.h"
#include "OffAxisLocalPlayer.generated.h"

/**
 *
 */
UCLASS()
class UOffAxisLocalPlayer : public ULocalPlayer
{
    GENERATED_BODY()

public:
    FSceneView *CalcSceneView(FSceneViewFamily *ViewFamily, FVector &OutViewLocation, FRotator &OutViewRotation,
                              FViewport *Viewport, FViewElementDrawer *ViewDrawer,
                              EStereoscopicPass StereoPass) override;

private:
    FMatrix GenerateOffAxisMatrix(const AMultiViewportCameraActor *Actor);

    //mapping cached view matrices to specific screen name
    UPROPERTY()
    TMap<FString, FMatrix> CachedViewMatrices;
};
