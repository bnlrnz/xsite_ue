// Fill out your copyright notice in the Description page of Project Settings.

#include "CaveGameModeBase.h"
#include "CaveSpectatorPawn.h"
#include "CavePlayerController.h"
#include "MultiViewportCameraActor.h"
#include <Developer/DesktopPlatform/Public/DesktopPlatformModule.h>
#include "JsonObjectConverter.h"
#include "JsonUtilities.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Engine.h"
#include "Misc/Paths.h"
#include "Engine.h"
#include "UObject/ConstructorHelpers.h"

ACaveGameModeBase::ACaveGameModeBase(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    DefaultPawnClass = ACaveSpectatorPawn::StaticClass();
    SpectatorClass = ACaveSpectatorPawn::StaticClass();
    PlayerControllerClass = ACavePlayerController::StaticClass();
}

void ACaveGameModeBase::PreLogin(const FString &Options, const FString &Address, const FUniqueNetIdRepl &UniqueId,
                                 FString &ErrorMessage)
{
    Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
    UE_LOG(LogNet, Display, TEXT("New player trying to connect. IP:%s"), *Address);
}

void ACaveGameModeBase::PostLogin(APlayerController *NewPlayer)
{
    Super::PostLogin(NewPlayer);
    UE_LOG(LogNet, Display, TEXT("New player added: %s"), *NewPlayer->GetName());
    PlayerControllerList.Add(NewPlayer);
}