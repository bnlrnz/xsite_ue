// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiViewportCameraActor.h"

#include "Engine.h"
#include "Engine/UserInterfaceSettings.h"
#include "Modules/ModuleManager.h"
#include "Blueprint/UserWidget.h"
#include "Slate/SceneViewport.h"
#include "Slate/SGameLayerManager.h"
#include "Components/SceneCaptureComponent2D.h"
#include "PNGImageLoader.h"

template <typename ObjClass>
static FORCEINLINE ObjClass* LoadObjFromPath(const FName& Path)
{
    if (Path == NAME_None) return nullptr;

    return Cast<ObjClass>(StaticLoadObject(ObjClass::StaticClass(), nullptr, *Path.ToString()));
}

static FORCEINLINE UMaterial* LoadMaterialFromPath(const FName& Path)
{
    if (Path == NAME_None) return nullptr;

    return LoadObjFromPath<UMaterial>(Path);
}

// Sets default values
AMultiViewportCameraActor::AMultiViewportCameraActor()
{
    SetActorLocation(FVector(0.0f, 0.0f, 0.0f));

    bReplicates = false;
    SetReplicateMovement(false);

    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    bAllowTickBeforeBeginPlay = false;

    // disable motion blur, it causes problems
    // disable temporal AA in project settings, if you get jittery screens!
    GetCameraComponent()->PostProcessSettings.MotionBlurAmount = 0.0f;

    BlendMaterial = LoadObjFromPath<UMaterial>(TEXT("Material'/xsite_ue/BlendMaterial.BlendMaterial'"));
    BlendMaterial->BlendablePriority = 1;

    BlendMaterialInstance_Dynamic = UMaterialInstanceDynamic::Create(BlendMaterial, this,
                                                                     TEXT("BlendMaterialInstance_Dynamic"));

    WarpMaterial = LoadObjFromPath<UMaterial>(TEXT("Material'/xsite_ue/WarpMaterial.WarpMaterial'"));
    WarpMaterial->BlendablePriority = 0;

    WarpMaterialInstance_Dynamic = UMaterialInstanceDynamic::Create(WarpMaterial, this,
                                                                    TEXT("WarpMaterialInstance_Dynamic"));

}

void AMultiViewportCameraActor::PreInitializeComponents()
{
    Super::PreInitializeComponents();

    // try cast to cave game instance
    CaveGameInstance = Cast<UCaveGameInstance>(GetWorld()->GetGameInstance());
    if (CaveGameInstance == nullptr)
    {
        UE_LOG(LogCave, Error, TEXT("Wrong Game Instance. Should be: UCaveGameInstance, was: %s"),
               *GetWorld()->GetGameInstance()->StaticClass()->GetFName().ToString());
    }
}

// Called when the game starts or when spawned
void AMultiViewportCameraActor::BeginPlay()
{
    // This shouldn't be reached in the first place if we are on the wrong client/computer
    // check whether we should spawn this screen on the current machine
    // AMultiViewportCameraActor should only be spawned on the correct clients,
    // Look at CaveController.cpp, there is almost the same statement
    bEnabledScreen =
        CaveGameInstance->GetComputerName().ToLower().Equals(this->ClientName.ToLower()) || 
        this->ClientName.Equals(FString("AllClients")) ||
        this->ClientName.Equals("*");

    // remove screen mesh from scene, we dont need it anymore
    if (!bEnabledScreen)
    {
        // kill this actor on the client, it is not needed
        Destroy();
        return;
    }

    // if we got here, the current screen is active -> message the server for logging
    ServerScreenAttached_Implementation(CaveGameInstance->GetComputerName(), WindowTitle.ToString());

    CamManager = nullptr;

    if (GetWorld())
    {
        // Get the player controller
        for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
        {
            APlayerController *PlayerControllerTemp = Iterator->Get();
            if (PlayerControllerTemp && PlayerControllerTemp->PlayerCameraManager)
            {
                this->PlayerController = PlayerControllerTemp;
                this->CamManager = this->PlayerController->PlayerCameraManager;
            }
        }
    }

    if (CamManager == nullptr)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red,
                                         FString(TEXT("ExtraCamWindow Error: No Player Camera Manager found!")));
        bEnabledScreen = false;
        return;
    }

    // this inits the Slate Renderer (that thing responsible for drawing the window)
    FSlateApplication::Get().GetRenderer();

    SAssignNew(ExtraWindow, SWindow)
        .ClientSize(InitialWindowRes)
        .SizingRule(ESizingRule::FixedSize)
        .UseOSWindowBorder(true)
        .Title(FText::FromString(FString::Printf(TEXT("%s - %s"), *WindowTitle.ToString(),
                                                 GetWorld()->IsServer() ? TEXT("Server") : TEXT("Client"))))
        .FocusWhenFirstShown(false)
        .CreateTitleBar(false);

    FSlateApplication::Get().AddWindow(ExtraWindow.ToSharedRef(), true);

    ViewportOverlayWidget = SNew(SOverlay);

    TSharedRef<SGameLayerManager> LayerManagerRef = SNew(SGameLayerManager);

    SGameLayerManager::FArguments arguments = SGameLayerManager::FArguments();
    arguments.SceneViewport(GEngine->GameViewport->GetGameViewport());
    arguments.Visibility(EVisibility::Visible);
    arguments.Cursor(EMouseCursor::None)[ViewportOverlayWidget.ToSharedRef()];

    LayerManagerRef.Get().Construct(arguments);
    LayerManagerRef.Get().SetVisibility(EVisibility::Visible);
    LayerManagerRef.Get().SetCursor(EMouseCursor::None);
    // LayerManagerRef.Get().UseScissor(false);

    TSharedPtr<SViewport> Viewport = SNew(SViewport)
                                         .RenderDirectlyToWindow(
                                             false) // true crashes some stuff because HMDs need the rendertarget tex for distortion etc..
                                         .EnableGammaCorrection(false)
                                         .EnableStereoRendering(false) // not displaying on an HMD
                                         .Cursor(EMouseCursor::None)[LayerManagerRef];

    SceneViewport = MakeShareable(new FSceneViewport(GEngine->GameViewport, Viewport));

    //UE_LOG(LogCave, Warning, TEXT("ViewportClient: %s"), *GEngine->GameViewportClientClassName.ToString());

    Viewport->SetViewportInterface(SceneViewport.ToSharedRef());

    ExtraWindow->SetContent(Viewport.ToSharedRef());
    ExtraWindow->MoveWindowTo(InitialWindowPos);
    ExtraWindow->SetOnWindowClosed(FOnWindowClosed::CreateLambda([this](const TSharedRef<SWindow> &window) {
#ifndef WITH_EDITOR
        // dont do this in editor, otherwise the editor will get killed too
        if (GetWorld()->IsPlayInEditor() || GetWorld()->IsPlayInPreview())
            return;

        ExtraWindow->RequestDestroyWindow();

        UE_LOG(LogCave, Error, TEXT("Window closed by user. Trying to tear down game instance..."));
        FGenericPlatformMisc::RequestExit(false);
#endif
    }));

    ExtraWindow->ShowWindow();

    SceneViewport->CaptureMouse(false);
    SceneViewport->SetUserFocus(false);
    SceneViewport->LockMouseToViewport(false);

    // the window and some stuff gets initialized by ticking slate, otherwise we get a thread-related crash in packaged builds..
    FSlateApplication::Get().Tick();

    UE_LOG(LogCave, Display, TEXT("[%s] Spawning Extra Window resolution: %s offset: %s"), *WindowTitle.ToString(), *InitialWindowRes.ToString(), *InitialWindowPos.ToString());

    // IdentifyScreen();
    
    if (this->PlayerController)
    {
        // r.HZBOcclusion 1 enables Hierarchical Z-Buffer Occlusion (see https://docs.unrealengine.com/en-US/Engine/Rendering/VisibilityCulling/index.html)
        // we do this method to prevent flickering actors (static meshes) caused by aggressive default frustum/occlusion culling
        // whenever an extra window is spawn (with a seperate view frustum than the "main" window) we should make shure to activate this
        // we could also set r.AllowOcclusionQueries=0, but this would probably disable culling all together (which we do not want?)
        this->PlayerController->ConsoleCommand("r.AllowOcclusionQueries 0");

        if (!GIsEditor) {
            //TODO: this is just for performance testing!
            // Edit from server with Cave_Execute "command"
            // https://docs.unrealengine.com/en-US/Engine/Performance/Scalability/ScalabilityReference/index.html
            this->PlayerController->ConsoleCommand("Gamma 5"); // our cave is currently really dark
            this->PlayerController->ConsoleCommand("r.DepthOfFieldQuality 0"); // we don't want that
            this->PlayerController->ConsoleCommand("r.ScreenPercentage 50"); // our cave walls smooth quite a lot of pixels
            this->PlayerController->ConsoleCommand("r.ssr.quality 0"); // flickering water reflections if we use ssr (screen space reflections)
            this->PlayerController->ConsoleCommand("r.PostProcessingAAQuality 1");
        }
        
        // if there is more than 1 render target (aka we spawning this extra window), this should be set https://docs.unrealengine.com/en-US/Platforms/MR/MRTroubleshooting/index.html
        this->PlayerController->ConsoleCommand("r.SceneRenderTargetResizeMethod 2");
    }

    // initialize everything before we call base class so that in blueprint beginplay everything is ready
    Super::BeginPlay();
}


void AMultiViewportCameraActor::ConfigureBlendMaterial(const ACaveControllerActor * const &CaveController, const FProjectorData& ScreenCalibrationData, const FWallData& WallCalibrationData){
    FString FullPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(
        *(CaveController->BlendTextureFolderPath.Path + ScreenCalibrationData.alphamask)
    );

    if (ScreenCalibrationData.alphamask.Len() == 0)
    {
        UE_LOG(LogCave, Warning, TEXT("[%s] No alphamask/blend texture set in calibration JSON."), *WindowTitle.ToString());
        bUseBlending = false;
        return;
    }
   
    //Apply the corresponding blend texture to the screen
    int32 Width;
    int32 Height;

    if (PNGImageLoader::FromFile(FullPath, BlendTexture, Width, Height))
    {

        bool applied = false;

        if (BlendMaterialInstance_Dynamic != nullptr)
        {
            TArray<FMaterialParameterInfo> ParameterInfos;
            TArray<FGuid> ParameterIds;

            BlendMaterialInstance_Dynamic->GetAllTextureParameterInfo(ParameterInfos, ParameterIds);

            for (const FMaterialParameterInfo &MaterialInfo : ParameterInfos)
            {
                if (MaterialInfo.Name.IsEqual("BlendTexture"))
                {
                    BlendMaterialInstance_Dynamic->SetTextureParameterValue(FName(TEXT("BlendTexture")), BlendTexture);
                    GetCameraComponent()->PostProcessSettings.AddBlendable(BlendMaterialInstance_Dynamic, 1.0f);
                    applied = true;
                    break;
                }
            }
        }
        if (!applied)
        {
            UE_LOG(LogCave, Error, TEXT("[%s] No alphamask/blend texture applied for screen. Texture Parameter 'BlendTexture' seams to be missing :( - BlendMaterial there?"), *WindowTitle.ToString());
        }
    }
    else
    {
        UE_LOG(LogCave, Error, TEXT("[%s] No alphamask/blend texture applied for screen. No texture path given or could not load texture (png) from filepath %s (Full Path: %s)"),
               *WindowTitle.ToString(), *ScreenCalibrationData.alphamask, *FullPath);
    }
}

void AMultiViewportCameraActor::ConfigureWarpMaterial(const FProjectorData& ScreenCalibrationData, const FWallData& WallCalibrationData){
    if (
        ScreenCalibrationData.corners.display_space.bottom_left.Num() == 0 ||
        ScreenCalibrationData.corners.display_space.bottom_right.Num() == 0 ||
        ScreenCalibrationData.corners.display_space.top_left.Num() == 0 ||
        ScreenCalibrationData.corners.display_space.top_right.Num() == 0 ||
        WallCalibrationData.camera.F.Num() == 0 ||
        ScreenCalibrationData.H_x.Num() == 0 ||
        ScreenCalibrationData.H_y.Num() == 0
    )
    {
        UE_LOG(LogCave, Warning, TEXT("[%s] All or some warp data missing. Skipping creation of warp shader..."), *WindowTitle.ToString());
        bUseWarping = false;
        return;
    }
    
    //Apply warp material
    //the current material is fragment/pixel based, which is not very efficient...
    //one could try a less accurate but faster solution. see: https://smus.com/vr-lens-distortion/
    if (WarpMaterialInstance_Dynamic != nullptr)
    {
        // TArray<FMaterialParameterInfo> ParameterInfos;
        // TArray<FGuid> ParameterIds;

        // WarpMaterialInstance_Dynamic->GetAllScalarParameterInfo(ParameterInfos, ParameterIds);
        const float sl[2]  = {0,1.0f};
        const float tl[2]  = {0,1.0f};

        const float left  = ScreenCalibrationData.corners.display_space.bottom_left[0] < ScreenCalibrationData.corners.display_space.top_left[0] ? ScreenCalibrationData.corners.display_space.bottom_left[0] :  ScreenCalibrationData.corners.display_space.top_left[0];
        const float bottom= ScreenCalibrationData.corners.display_space.bottom_left[1] < ScreenCalibrationData.corners.display_space.bottom_right[1] ? ScreenCalibrationData.corners.display_space.bottom_left[1] :  ScreenCalibrationData.corners.display_space.bottom_right[1];
        const float right = ScreenCalibrationData.corners.display_space.bottom_right[0] > ScreenCalibrationData.corners.display_space.top_right[0] ? ScreenCalibrationData.corners.display_space.bottom_right[0] :  ScreenCalibrationData.corners.display_space.top_right[0];
        const float top   = ScreenCalibrationData.corners.display_space.top_right[1] > ScreenCalibrationData.corners.display_space.top_left[1] ? ScreenCalibrationData.corners.display_space.top_right[1] :  ScreenCalibrationData.corners.display_space.top_left[1];

        const float s[2] = {left, right}; 
        const float t[2] = {top, bottom}; 

        const float cx = (sl[0] - sl[1]) / (s[0] - s[1]);
        const float cy = (tl[0] - tl[1]) / (t[0] - t[1]);
        const float tx = sl[0] - cx * s[0];
        const float ty = tl[0] - cy * t[0];

        const double Fscaled[9] = {
            WallCalibrationData.camera.F[0]*cx + WallCalibrationData.camera.F[6]*tx, WallCalibrationData.camera.F[1]*cx + WallCalibrationData.camera.F[7]*tx, WallCalibrationData.camera.F[2]*cx + WallCalibrationData.camera.F[8]*tx,
            WallCalibrationData.camera.F[3]*cy + WallCalibrationData.camera.F[6]*ty, WallCalibrationData.camera.F[4]*cy + WallCalibrationData.camera.F[7]*ty, WallCalibrationData.camera.F[5]*cy + WallCalibrationData.camera.F[8]*ty,
            WallCalibrationData.camera.F[6],                  WallCalibrationData.camera.F[7],                  WallCalibrationData.camera.F[8]
        };
        
        for (int i = 0; i < 9; ++i)
        {
            FString Key = FString::Printf(TEXT("F%d"), i);
            //UE_LOG(LogCave, Warning, TEXT("[%s-%s] %sscaled = %.16f"), *ClientName, *WindowTitle.ToString(), *Key, Fscaled[i]);
            //UE_LOG(LogCave, Warning, TEXT("[%s-%s] %s = %.16f"), *ClientName, *WindowTitle.ToString(), *Key, WallCalibrationData.F[i]);
            WarpMaterialInstance_Dynamic->SetScalarParameterValue(FName(*Key), Fscaled[i]);
        } 

        for (int i = 0; i < 20; ++i)
        {
            FString Key = FString::Printf(TEXT("H%d"), i);
            //UE_LOG(LogCave, Warning, TEXT("[%s-%s] %s = %.16f"), *ClientName, *WindowTitle.ToString(), *Key, ScreenCalibrationData.H[i]);
            WarpMaterialInstance_Dynamic->SetScalarParameterValue(FName(*Key), i < 10 ? ScreenCalibrationData.H_x[i] : ScreenCalibrationData.H_y[i-10]);
        }

        UE_LOG(LogCave, Display, TEXT("[%s] Applied Warp Data"), *WindowTitle.ToString());

        GetCameraComponent()->PostProcessSettings.AddBlendable(WarpMaterialInstance_Dynamic, 1.0f);
    }

}

void AMultiViewportCameraActor::ConfigurePostProcessingMaterials(const ACaveControllerActor * const &CaveController, const FProjectorData& ScreenCalibrationData, const FWallData& WallCalibrationData)
{
    ConfigureBlendMaterial(CaveController, ScreenCalibrationData, WallCalibrationData);
    ConfigureWarpMaterial(ScreenCalibrationData, WallCalibrationData);
}

void AMultiViewportCameraActor::ServerScreenAttached_Implementation(const FString &ComputerName, const FString &ScreenName)
{
    if (GetWorld()->IsServer())
        GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Green,
                                         FString::Printf(TEXT("[%s] Screen Attached: %s"), *WindowTitle.ToString(), *ScreenName));
}

bool AMultiViewportCameraActor::ServerScreenAttached_Validate(const FString &ComputerName, const FString &ScreenName)
{
    // nothing to validate here, but validation is required :(
    return true;
}

void AMultiViewportCameraActor::IdentifyScreen()
{
    if (IsValid(TextBox))
    {
        RemoveWidgetFromExtraCam(TextBox);
    }

    // remove all old widgets
    ViewportOverlayWidget->ClearChildren();

    // build text widget
    TextBox = CreateWidget<UTextBoxWidget>(PlayerController, UTextBoxWidget::StaticClass());
    TextBox->ViewportOffset = FVector2D(InitialWindowRes.X / 2, InitialWindowRes.Y / 2);
    TextBox->Text = FString::Printf(TEXT("%s\n%s"), *WindowTitle.ToString(),
                                    GetWorld()->IsServer() ? TEXT("Server") : TEXT("Client"));
    
    // add text widget to screen
    AddWidgetToExtraCam(TextBox, 999);
    
    bScreenIdentifyEnabled = true;
}

void AMultiViewportCameraActor::IdentifyScreenClear()
{
    if (IsValid(TextBox))
    {
        RemoveWidgetFromExtraCam(TextBox);
    }

    // remove all old widgets
    ViewportOverlayWidget->ClearChildren();

    bScreenIdentifyEnabled = false;
}

bool AMultiViewportCameraActor::AddWidgetToExtraCam(UUserWidget *inWidget, int32 zOrder /* = -1 */)
{
    if (!ViewportOverlayWidget.IsValid())
        return false;

    ViewportOverlayWidget->AddSlot(zOrder)[inWidget->TakeWidget()];

    return true;
}

bool AMultiViewportCameraActor::RemoveWidgetFromExtraCam(UUserWidget *inWidget)
{
    if (!ViewportOverlayWidget.IsValid())
        return false;

    return ViewportOverlayWidget->RemoveSlot(inWidget->TakeWidget());
}

void AMultiViewportCameraActor::Tick(float delta)
{
    Super::Tick(delta);

    // this should not happen at this point, but just to be shure
    if (!bEnabledScreen)
        return;
   
    // if we got here, we dont need the default window anymore
    if (!GIsEditor)
        CaveGameInstance->Cave_HideDefaultWindow();

    if (bScreenIdentifyEnabled)
    {
        if (IsValid(TextBox))
        {
            TextBox->TextBlock->SetText(
                FText::FromString(FString::Printf(TEXT("%s\n%s\n%.2f FPS"),
                    *WindowTitle.ToString(),
                    GetWorld()->IsServer() ? TEXT("Server") : TEXT("Client"),
                    1.0f/delta))
            );
        }
    }

    // replicating the rotation and location via net multicast rpc from server to the clients seems to work fine
    SetActorLocation(CaveGameInstance->GetCaveHeadCharacter()->NetLoc);
    SetActorRotation(CaveGameInstance->GetCaveHeadCharacter()->NetRot);

    // check if we should warp
    if (bUseWarping && WarpMaterialInstance_Dynamic != nullptr && !bWarpingEnabled)
    {
        bWarpingEnabled = true;
        GetCameraComponent()->PostProcessSettings.AddBlendable(WarpMaterialInstance_Dynamic, 1.0f);
    }

    if (!bUseWarping && WarpMaterialInstance_Dynamic != nullptr && bWarpingEnabled)
    {
        bWarpingEnabled = false;
        GetCameraComponent()->PostProcessSettings.RemoveBlendable(WarpMaterialInstance_Dynamic);
    }

    // check if we should blend
    if (bUseBlending && BlendMaterialInstance_Dynamic != nullptr && !bBlendingEnabled)
    {
        bBlendingEnabled = true;
        GetCameraComponent()->PostProcessSettings.AddBlendable(BlendMaterialInstance_Dynamic, 1.0f);
    }

    if (!bUseBlending && BlendMaterialInstance_Dynamic != nullptr && bBlendingEnabled)
    {
        bBlendingEnabled = false;
        GetCameraComponent()->PostProcessSettings.RemoveBlendable(BlendMaterialInstance_Dynamic);
    }

    // adjust camera to fit this actor
    AActor *oldTarget = nullptr;
    oldTarget = CamManager->GetViewTarget();

    CamManager->SetViewTarget(this);
    CamManager->UpdateCamera(0.0f);

    SceneViewport->Draw();

    // reset camera to player camera
    CamManager->SetViewTarget(oldTarget);
}

void AMultiViewportCameraActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    if (!bEnabledScreen)
        return;

    if (IsValid(TextBox))
    {
        RemoveWidgetFromExtraCam(TextBox);
    }

    if (ViewportOverlayWidget.IsValid())
    {
        ViewportOverlayWidget->ClearChildren();
        ViewportOverlayWidget.Reset();
    }

    if (SceneViewport.IsValid())
        SceneViewport.Reset();
    
    if (ExtraWindow.IsValid())
    {
        ExtraWindow->RequestDestroyWindow();
    }
}