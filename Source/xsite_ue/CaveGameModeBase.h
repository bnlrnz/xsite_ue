// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CaveGameInstance.h"
#include "CaveGameModeBase.generated.h"

UCLASS()
class  ACaveGameModeBase : public AGameModeBase {
    GENERATED_BODY()

public:
    ACaveGameModeBase(const FObjectInitializer &ObjectInitializer);

    virtual void PreLogin(const FString &Options, const FString &Address, const FUniqueNetIdRepl &UniqueId,
                          FString &ErrorMessage) override;

    virtual void PostLogin(APlayerController *NewPlayer) override;

    // List of PlayerControllers
    TArray<class APlayerController *> PlayerControllerList;
};