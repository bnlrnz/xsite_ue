// Fill out your copyright notice in the Description page of Project Settings.

#include "CaveControllerActor.h"
#include "JsonObjectConverter.h"
#include "JsonUtilities.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "MultiViewportCameraActor.h"
#include "VRPNControllerActor.h"
#include "CaveGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "CaveCalibrationParser.h"

#define UE_SCALE_FACTOR 100

// Sets default values
ACaveControllerActor::ACaveControllerActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;

    SetReplicates(true);
}

// Called when the game starts or when spawned
void ACaveControllerActor::BeginPlay()
{
    Super::BeginPlay();

    LoadWallScreenConfig();
}

void ACaveControllerActor::LoadWallScreenConfig()
{
    // trying to load up Calibration json:
    FString CalibrationJson;

    FString File = this->ConfigurationFilePath.FilePath;

    FString ConfigurationFileJustPath;
    FString ConfigurationFileName;
    FString ConfigurationFileExtension;

    FPaths::Split(File, ConfigurationFileJustPath, ConfigurationFileName, ConfigurationFileExtension);

    FString FullPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*File);

    if (!FFileHelper::LoadFileToString(CalibrationJson, *File))
    {
        UE_LOG(LogCave, Error, TEXT("Could not load calibration file (json) from %s (absolute path: %s)"), *File, *FullPath);

        // Try this fallback:
        //InstallDir/LinuxNoEditor/GameName/Content
        File = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()) + "Calibration.json";
        FullPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*File);

        UE_LOG(LogCave, Error, TEXT("Trying fallback calibration file path... (%s)"), *FullPath);
        
        if (!FFileHelper::LoadFileToString(CalibrationJson, *File))
        {
            UE_LOG(LogCave, Error, TEXT("Fallback: Could not load calibration file (json) from %s (absolute path: %s)"), *File, *FullPath);
            return;
        }
        else
        {
            UE_LOG(LogCave, Display, TEXT("Able to load fallback calibration file (json) from %s (absolute path: %s)"), *File, *FullPath);
        }
    }
    else
    {
        UE_LOG(LogCave, Display, TEXT("Able to load calibration file (json) from %s (absolute path: %s)"), *File, *FullPath);
    }

    FCalibrationData CalibrationData;

    if (!CaveCalibrationParser::ParseJsonString(CalibrationJson, &CalibrationData))
    {
        UE_LOG(LogCave, Error, TEXT("Could not parse calibration file (json) from %s"), *File);
        return;
    }

    auto CaveGameInstance = Cast<UCaveGameInstance>(GetWorld()->GetGameInstance());

    if (CalibrationData.eye.Num() != 3)
    {
        UE_LOG(LogCave, Error, TEXT("There is no eye origin defined in calibration file (json), falling back to 0,0,0."));
        EyeOrigin = FVector(0,0,0);
    }else
    {
        EyeOrigin = FVector(-CalibrationData.eye[2], CalibrationData.eye[0], CalibrationData.eye[1]);
    }
    
    if (CalibrationData.walls.Num() == 0)
    {
        UE_LOG(LogCave, Error, TEXT("There are no walls defined in calibration file (json)"));
    }

    for (auto Wall : CalibrationData.walls)
    {
        if (Wall.clients.Num() == 0)
        {
            UE_LOG(LogCave, Error, TEXT("There are no clients/screens defined in calibration file (json) for wall %s"), *Wall.name);
        }

        for (auto Client : Wall.clients)
        {
            UE_LOG(LogCave, Display, TEXT("Found Client %s-%d for wall %s"), *Client.name, Client.id, *Wall.name);

            if (CaveGameInstance != nullptr && !CaveGameInstance->GetComputerName().Equals(Client.name))
            {
                UE_LOG(LogCave, Display, TEXT("Skipping Client... not my business"));
                continue;
            }

            for (auto Proj : Client.projectors)
            {
                // GameMode is only present on server
                // every client will spawn its own OffAxisCameraExtraWindowActors
                // the clients will skip all OffAxisCameraExtraWindowActors but their own

                auto OffAxisCameraExtraWindowActor = GetWorld()->SpawnActorDeferred<AMultiViewportCameraActor>(AMultiViewportCameraActor::StaticClass(), FTransform());

                OffAxisCameraExtraWindowActor->ClientName = Client.name;
                OffAxisCameraExtraWindowActor->WindowTitle = FText::FromString(FString::Printf(TEXT("%s-%s-%s"), *Wall.name, *Client.name, *Proj.name));
                OffAxisCameraExtraWindowActor->InitialWindowPos = FVector2D(Proj.offset[0], Proj.offset[1]);
                OffAxisCameraExtraWindowActor->InitialWindowRes = FVector2D(Proj.resolution[0], Proj.resolution[1]);

                // Set Proj parameter
                // ltrb in relation to the wall
                const float left = Proj.corners.display_space.bottom_left[0] < Proj.corners.display_space.top_left[0] ? Proj.corners.display_space.bottom_left[0] : Proj.corners.display_space.top_left[0];
                const float bottom = Proj.corners.display_space.bottom_left[1] < Proj.corners.display_space.bottom_right[1] ? Proj.corners.display_space.bottom_left[1] : Proj.corners.display_space.bottom_right[1];
                const float right = Proj.corners.display_space.bottom_right[0] > Proj.corners.display_space.top_right[0] ? Proj.corners.display_space.bottom_right[0] : Proj.corners.display_space.top_right[0];
                const float top = Proj.corners.display_space.top_right[1] > Proj.corners.display_space.top_left[1] ? Proj.corners.display_space.top_right[1] : Proj.corners.display_space.top_left[1];

                // Scaled by 100 to match unreal engine size
                // 1cm == 1UU (UnrealEngine Unit)
                double WallWidth = Wall.size[0] * UE_SCALE_FACTOR;
                double WallHeight = Wall.size[1] * UE_SCALE_FACTOR;

                OffAxisCameraExtraWindowActor->ScreenSize = FVector2D((right - left) * WallWidth, (bottom - top) * WallHeight);

                // "name" : "Wall_Front",
                // "size" : [3.0, 3.0],
                // "normal" : [0.0, 0.0, 1.0],
                // "bounds" : {
                // 	"top_left" : [-1.5, 3.0, -2.065],
                // 	"bottom_right" : [1.5, 0.0, -2.065]
                // },
                //
                // Cave Coordinate System is setup so that:
                // -Z is Front      --> X in Unreal
                // X is Left/Right  --> Y in Unreal
                // Y is Up/Down     --> Z in Unreal

                // What I currently got
                // Wall tl: X=-150.000 Y=300.000 Z=-206.500
                // but should be in Unreal Coord System:
                // Wall tl: X=206.500 Y=-150.000 Z=300.000

                const auto normal = FVector(
                                        -Wall.normal[2],
                                        Wall.normal[0],
                                        Wall.normal[1])
                                        .GetUnsafeNormal(); // just to be shure

                // Top Left WallCoord
                const auto tl = UE_SCALE_FACTOR * FVector(
                                                      -Wall.bounds.top_left[2],
                                                      Wall.bounds.top_left[0],
                                                      Wall.bounds.top_left[1]);

                // Bottom Right WallCoord
                const auto br = UE_SCALE_FACTOR * FVector(
                                                      -Wall.bounds.bottom_right[2],
                                                      Wall.bounds.bottom_right[0],
                                                      Wall.bounds.bottom_right[1]);

                // angle "at" tl
                const double alpha = UKismetMathLibrary::Atan(WallHeight / WallWidth);

                // orth length from tl_br to tr / bl
                const double r = FMath::Sin(alpha) * WallWidth;

                // length on tl_br
                const double t = FMath::Cos(alpha) * WallWidth;

                // normalized vector from top left to bottom right (diagonal)
                const auto Norm_tl_br = (br - tl).GetUnsafeNormal();

                // normalized vector orth to tl_br and the normal of the plane/screen
                const auto Norm_R = FVector::CrossProduct(Norm_tl_br, normal).GetUnsafeNormal();

                const auto tr = tl + t * Norm_tl_br + r * Norm_R;
                const auto bl = tl + ((br - tl).Size() - t) * Norm_tl_br - r * Norm_R;

                const auto Norm_tl_tr = (tr - tl).GetUnsafeNormal();
                const auto Norm_tl_bl = (bl - tl).GetUnsafeNormal();

                // We now got the Wall extents and normalized vetors from wall origin to the edges
                // We kann now calculate the screen center in wall/world coordinates from the display space corners
                // Display Space Corners -> relative corners of a screen in relation to its wall
                //	TopLeft, TopRight, BottomRight, BottomLeft
                //
                // example: (0,0), (0.5,0), (0.5,0.3), (0,0.3)
                //
                //		x-------x-------|
                //		|Screen |		|
                //		x-------x-------|
                //		|		|		|
                //		|---------------|
                //		|		|		|
                //		|---------------|
                //			   Wall

                const auto Screen_tl = tl + Norm_tl_tr * left * WallWidth + Norm_tl_bl * top * WallHeight;
                const auto Screen_br = tl + Norm_tl_tr * right * WallWidth + Norm_tl_bl * bottom * WallHeight;
                const auto Screen_bl = tl + Norm_tl_tr * left * WallWidth + Norm_tl_bl * bottom * WallHeight;

                // UE_LOG(LogCave, Warning, TEXT("%s\ntl: %s\nbr: %s\nbl: %s\nScreen_tl: %s\nScreen_br: %s\nScreen_bl: %s\n"),
                //     *OffAxisCameraExtraWindowActor->WindowTitle.ToString(),
                //     *tl.ToString(),
                //     *br.ToString(),
                //     *bl.ToString(),
                //     *Screen_tl.ToString(),
                //     *Screen_br.ToString(),
                //     *Screen_bl.ToString()
                // );

                OffAxisCameraExtraWindowActor->ScreenRotation = UKismetMathLibrary::MakeRotFromX(normal);
                OffAxisCameraExtraWindowActor->ScreenTopLeft = FVector(Screen_tl);
                OffAxisCameraExtraWindowActor->ScreenBottomLeft = FVector(Screen_bl);
                OffAxisCameraExtraWindowActor->ScreenBottomRight = FVector(Screen_br);
                OffAxisCameraExtraWindowActor->ConfigurePostProcessingMaterials(this, Proj, Wall);

                UGameplayStatics::FinishSpawningActor(OffAxisCameraExtraWindowActor, FTransform());

                UE_LOG(LogCave, Display, TEXT("[%s] Finishing Actor spawn, size: %s, screen top left: %s\n\n"), *OffAxisCameraExtraWindowActor->WindowTitle.ToString(), *Wall.name, *OffAxisCameraExtraWindowActor->ScreenSize.ToString(), *Screen_tl.ToString());
                //}
            }
        }
    }
}