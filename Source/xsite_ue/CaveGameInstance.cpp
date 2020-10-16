// Fill out your copyright notice in the Description page of Project Settings.

#include "CaveGameInstance.h"
#include "Engine.h"

FString UCaveGameInstance::GetComputerName() { return FString(FPlatformProcess::ComputerName()); }

ACaveHeadCharacter *UCaveGameInstance::GetCaveHeadCharacter()
{
    if (CachedCaveHeadCharacter != nullptr)
        return CachedCaveHeadCharacter;

    // there should only be one Cave Head per game instance
    if (GetWorld())
    {
        for (TActorIterator<ACaveHeadCharacter> CharacterItr(GetWorld()); CharacterItr; ++CharacterItr)
        {
            CachedCaveHeadCharacter = *CharacterItr;
            return CachedCaveHeadCharacter;
        }
    }

    return nullptr;
}

ACaveControllerActor *UCaveGameInstance::GetCaveController()
{
    if (CachedCaveController != nullptr)
        return CachedCaveController;

    if (GetWorld())
    {
        for (TActorIterator<ACaveControllerActor> CntrItr(GetWorld()); CntrItr; ++CntrItr)
        {
            CachedCaveController = *CntrItr;
            return CachedCaveController;
        }
    }

    return nullptr;
}

AVRPNControllerActor* UCaveGameInstance::GetVRPNControllerActor(const FString& DeviceName)
{
    if (!GetWorld()->IsServer())
        return nullptr;

    if (GetWorld())
    {
        for (TActorIterator<AVRPNControllerActor> CntrItr(GetWorld()); CntrItr; ++CntrItr)
        {
            auto *VRPNControllerActor = *CntrItr;
            
            if (VRPNControllerActor->VRPN_Device_Name.Compare(DeviceName) == 0)
            {
                return *CntrItr;
            }
        }
    }

    UE_LOG(LogCave, Error, TEXT("There is no VRPNControllerActor with name '%s'. Did you place and setup the VRPNControllerActor correctly?"), *DeviceName);

    return nullptr;
}


void UCaveGameInstance::Cave_Shutdown()
{
    ACaveHeadCharacter *CaveHeadCharacter = GetCaveHeadCharacter();

    if (CaveHeadCharacter == nullptr)
        return;

    CaveHeadCharacter->ExitGame();
}

void UCaveGameInstance::Cave_IdentifyScreens()
{
    ACaveHeadCharacter *CaveHeadCharacter = GetCaveHeadCharacter();

    if (CaveHeadCharacter == nullptr)
        return;

    CaveHeadCharacter->Multicast_IdentifyScreen();
}

void UCaveGameInstance::Cave_IdentifyScreensClear()
{
    ACaveHeadCharacter *CaveHeadCharacter = GetCaveHeadCharacter();

    if (CaveHeadCharacter == nullptr)
        return;

    CaveHeadCharacter->Multicast_IdentifyScreenClear();
}

void UCaveGameInstance::Cave_Execute(const FString &Command)
{
    auto CaveHeadCharacter = GetCaveHeadCharacter();
    
    if (CaveHeadCharacter == nullptr)
        return;

    CaveHeadCharacter->Multicast_ExecuteCommand(Command);
}

void UCaveGameInstance::Cave_SetWarping(const FString &hostname, bool enabled)
{
    ACaveHeadCharacter *CaveHeadCharacter = GetCaveHeadCharacter();

    if (CaveHeadCharacter == nullptr)
        return;

    CaveHeadCharacter->Multicast_Warping(hostname, enabled);
}

void UCaveGameInstance::Cave_SetBlending(const FString &hostname, bool enabled)
{
    ACaveHeadCharacter *CaveHeadCharacter = GetCaveHeadCharacter();

    if (CaveHeadCharacter == nullptr)
        return;

    CaveHeadCharacter->Multicast_Blending(hostname, enabled);
}

void UCaveGameInstance::Cave_Map(const FString& Map)
{
    // TODO: this does not work for the clients :(
    GetWorld()->ServerTravel(*Map, ETravelType::TRAVEL_Absolute);
}

void UCaveGameInstance::Cave_SetWarpingAll(bool enabled)
{
    ACaveHeadCharacter *CaveHeadCharacter = GetCaveHeadCharacter();

    if (CaveHeadCharacter == nullptr)
        return;

    CaveHeadCharacter->Multicast_WarpingAll(enabled);
}

void UCaveGameInstance::Cave_SetBlendingAll(bool enabled)
{
    ACaveHeadCharacter *CaveHeadCharacter = GetCaveHeadCharacter();

    if (CaveHeadCharacter == nullptr)
        return;

    CaveHeadCharacter->Multicast_BlendingAll(enabled);
}

