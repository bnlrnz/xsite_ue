// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CaveHeadCharacter.h"
#include "CaveControllerActor.h"
#include "CaveGameInstance.generated.h"

UCLASS()
class  UCaveGameInstance : public UGameInstance {
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    FString GetComputerName();

    UFUNCTION(BlueprintCallable)
    ACaveHeadCharacter* GetCaveHeadCharacter();

    UFUNCTION(BlueprintCallable)
    ACaveControllerActor* GetCaveController();

    UFUNCTION(BlueprintCallable)
    AVRPNControllerActor* GetVRPNControllerActor(const FString& DeviceName);

    UFUNCTION()
    void ListAllCameraActors();

    /*console command to execute console commands on server and clients*/
    UFUNCTION(Exec, Category = Cave)
    void Cave_Execute(const FString &Command);

    /*console command to identify the host screens*/
    UFUNCTION(Exec, Category = Cave)
    void Cave_IdentifyScreens();

    /*console command to clear the identify message*/
    UFUNCTION(Exec, Category = Cave)
    void Cave_IdentifyScreensClear();

    /*console command to exit all server and clients*/
    UFUNCTION(Exec, Category = Cave)
    void Cave_Shutdown();

    /*console command to set warping on a host*/
    UFUNCTION(Exec, Category = Cave)
    void Cave_SetWarping(const FString &hostname, bool enabled);

    /*console command to set blending on a host*/
    UFUNCTION(Exec, Category = Cave)
    void Cave_SetBlending(const FString &hostname, bool enabled);

    /*console command to swich map*/
    UFUNCTION(Exec, Category = Cave)
    void Cave_Map (const FString& Map);

    /*console command to set warping for all connected clients*/
    UFUNCTION(Exec, Category = Cave)
    void Cave_SetWarpingAll(bool enabled);

    /*console command to set blending for all connected clients*/
    UFUNCTION(Exec, Category = Cave)
    void Cave_SetBlendingAll(bool enabled);

    private:
        UPROPERTY()
        ACaveHeadCharacter* CachedCaveHeadCharacter = nullptr;
        
        UPROPERTY()
        ACaveControllerActor* CachedCaveController = nullptr;
};


