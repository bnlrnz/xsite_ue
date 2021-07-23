// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshSocket.h"
#include "CaveHeadCharacter.h"
#include "VRPNFlystickActor.generated.h"

UENUM(BlueprintType)
enum InteractionMode
{
    Manipulate,
    Navigate
};

UCLASS()
class AVRPNFlystickActor : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AVRPNFlystickActor();

    UPROPERTY(BlueprintReadOnly)
    TEnumAsByte<InteractionMode> FlystickMode = Manipulate;

    UPROPERTY(BlueprintReadOnly)
    TEnumAsByte<ButtonState> PrimaryButtonState = ButtonState::Released;

    UFUNCTION(BlueprintImplementableEvent, Category = "Flystick")
    void OnPrimaryButtonStateChanged(ButtonState NewState);
    void OnPrimaryButtonStateChanged_Implementation(ButtonState NewState);

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_UpdatePointer(FVector NetStart, FVector NetEnd, bool NetbIsHit);

    UPROPERTY(BlueprintReadOnly)
    FVector Start = FVector(50.0f, 0.0f, 0.0f);

    UPROPERTY(BlueprintReadOnly)
    FVector End = FVector(205.0f, 0.0f, 0.0f);

    bool bIsHit = false;

    UPROPERTY(BlueprintReadWrite)
    FString AttachedActorName = TEXT("");

    UPROPERTY(BlueprintReadOnly)
    AActor *PointedActor = nullptr;

private:
    bool bShowLaser = true;

    bool bPrimaryClick = false;

    double AnalogX = 0.0;
    double AnalogY = 0.0;
    double LaserOffset = 0.0;

    UPROPERTY()
    ACaveHeadCharacter *CaveHeadCharacter = nullptr;

    FRotator CaveHeadCharacterStartRotation = FRotator(0, 0, 0);
    FRotator DragStartRotation = FRotator(0, 0, 0);

    FVector PlayerStartLocation = FVector(0,0,0);

    void DeselectPointedActor();

    void HandleManipulate();
    void HandleNavigate();

    // TODO: Make this configurable from json (the walls are defined there!)
    const FVector LeftWallPosition = FVector(0.0f, -148.75f, 0.0f);
    const FVector LeftWallNormal = FVector(0.0f, 1.0f, 0.0f);

    const FVector FrontWallPosition = FVector(205.0f, 0.0f, 0.0f);
    const FVector FrontWallNormal = FVector(-1.0f, 0.0f, 0.0f);

    const FVector RightWallPosition = FVector(0.0f, 148.75f, 0.0f);
    const FVector RightWallNormal = FVector(0.0f, -1.0f, 0.0f);

    const FVector FloorPosition = FVector(0.0f, 0.0f, -130.0f);
    const FVector FloorNormal = FVector(0.0f, 0.0f, 1.0f);

    TArray<FVector> WallPositions;
    TArray<FVector> WallNormals;
};
