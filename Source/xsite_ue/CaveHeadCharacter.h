// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "xsite_ue.h"
#include "Components/SphereComponent.h"
#include "VRPNController.h"
#include "CaveHeadCharacter.generated.h"

UCLASS()
class  ACaveHeadCharacter : public ACharacter {
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    ACaveHeadCharacter();

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;

    // define additional replicated props
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

    // indicates whether headtracking is enabled or not
    UPROPERTY(Replicated)
    bool bHeadtrackingEnabled = false;

    FVector GetRealHeadLocation();

    // I tried normal replication, movement replication, smoothing on client side... but always got jaggy rotation replication
    // Doing a simple rpc seems to work fine here. At least on a LAN (which is our use case)
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_UpdateLocationAndRotation(FVector Loc, FRotator Rot);

    FVector NetLoc = FVector(0,0,0);
    FRotator NetRot = FRotator(0,0,0);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_ExecuteCommand(const FString &Command);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_IdentifyScreen();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_Warping(const FString &hostname, bool enabled);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_Blending(const FString &hostname, bool enabled);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_WarpingAll(bool enabled);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_BlendingAll(bool enabled);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_IdentifyScreenClear();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_ExitGame();

    void ExitGame();

    virtual void Tick( float DeltaTime) override;

    FVector GetPlayerStartLocation() { return PlayerStartLocation; }
    FRotator GetPlayerStartRotation() { return PlayerStartRotation; }

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

private:
    UPROPERTY(Replicated)
    FVector HeadOrigin = FVector(0.0f, 0.0f ,0.0f);

    UPROPERTY(Replicated)
    FVector PlayerStartLocation= FVector(0.0f, 0.0f, 0.0f);

    UPROPERTY(Replicated)
    FRotator PlayerStartRotation = FRotator(0.0f, 0.0f, 0.0f);

    enum INPUT_MODE {
        MOVE, SETUP /* SETUP is deprecated*/
    } eInputMode = MOVE;

    void ToggleHeadtracking();

    void ToggleGhost();

    void ToggleFPS();

    bool bIsGhost = false;

    void ResetHead();

    void PrintHelp();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_ToggleGhost();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_ToggleFPS();

    float SpeedFactor = 1.0f;

    void SlowEnabled();

    void SlowDisabled();

    // handles moving head forward - backward / screen up - down
    void MoveForward(float Val);

    // handles strafing
    void StrafeRight(float Val);

    // handle moving head up - down / screen forward - backward
    void MoveUp(float Val);

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
