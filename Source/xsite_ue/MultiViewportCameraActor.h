// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "CaveGameInstance.h"
#include "ProceduralMeshComponent.h"
#include "CaveGameModeBase.h"
#include "CaveControllerActor.h"
#include "TextBoxWidget.h"
#include "MultiViewportCameraActor.generated.h"

/**
 *
 */
UCLASS()
class  AMultiViewportCameraActor : public ACameraActor {
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AMultiViewportCameraActor();

    // Displays text on screen to identify hostname, window and also shows framerate (fps)
    void IdentifyScreen();
    void IdentifyScreenClear();
    bool bScreenIdentifyEnabled = false;

    UPROPERTY()
    FString ClientName = "";

    bool bEnabledScreen = true;

    // called on client executed on server
    UFUNCTION( Server, Unreliable, WithValidation )
    void ServerScreenAttached(const FString& ComputerName, const FString& ScreenName);

    UPROPERTY()
    FText WindowTitle = FText::FromString(TEXT("ExtraCamWindow"));

    UPROPERTY()
    FVector2D InitialWindowRes = FVector2D(640, 360);

    UPROPERTY()
    FVector2D InitialWindowPos = FVector2D(0, 0);

    UFUNCTION()
    bool AddWidgetToExtraCam(UUserWidget *inWidget, int32 zOrder = -1);

    UFUNCTION()
    bool RemoveWidgetFromExtraCam(UUserWidget *inWidget);

    // The screen bounds
    UPROPERTY()
    FVector ScreenTopLeft;

    UPROPERTY()
    FVector ScreenBottomRight;

    UPROPERTY()
    FVector ScreenBottomLeft;

    UPROPERTY()
    FVector2D ScreenSize;

    UPROPERTY()
    FRotator ScreenRotation;

    // this determines if the view matrix should be recalculated
    // this should be true on the first run, when head tracking is enabled and everytime the view/screen/projector bounds change (usually not)
    bool bShouldCalculateViewMatrix = true;
    
    // must be called before BeginPlay() !
    // this will set up the warping and blending
    void ConfigurePostProcessingMaterials(const ACaveControllerActor * const & CaveController, const FProjectorData& ScreenCalibrationData, const FWallData& WallCalibrationData);
    void ConfigureBlendMaterial(const ACaveControllerActor * const & CaveController, const FProjectorData& ScreenCalibrationData, const FWallData& WallCalibrationData);
    void ConfigureWarpMaterial(const FProjectorData& ScreenCalibrationData, const FWallData& WallCalibrationData);

    ///////////////////
    // Blending related
    ///////////////////

    UPROPERTY()
    UMaterial *BlendMaterial;

    UPROPERTY(Transient)
    UMaterialInstanceDynamic *BlendMaterialInstance_Dynamic{};

    UPROPERTY(EditAnywhere)
    bool bUseBlending = true;

    UPROPERTY()
    UTexture2D *BlendTexture{};

    ///////////////////
    // Warping related
    ///////////////////

    UPROPERTY()
    UMaterial *WarpMaterial;

    UPROPERTY(Transient)
    UMaterialInstanceDynamic *WarpMaterialInstance_Dynamic{};

    UPROPERTY(EditAnywhere)
    bool bUseWarping = true;

private:
    bool bWarpingEnabled = true;
    bool bBlendingEnabled = true;

    void BeginPlay() override;
    void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    void Tick(float delta) override;

    void PreInitializeComponents() override;

    TSharedPtr<FSceneViewport> SceneViewport = nullptr;
    TSharedPtr<SOverlay> ViewportOverlayWidget = nullptr;
    TSharedPtr<SWindow> ExtraWindow = nullptr;
    
    UPROPERTY()
    UTextBoxWidget* TextBox = nullptr;

    UPROPERTY()
    APlayerController *PlayerController = nullptr;
    
    UPROPERTY()
    APlayerCameraManager *CamManager = nullptr;

    UPROPERTY()
    UCaveGameInstance *CaveGameInstance = nullptr;
};
