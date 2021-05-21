// Fill out your copyright notice in the Description page of Project Settings.

#include "CaveSpectatorPawn.h"
#include "Engine.h"
#include "CaveGameModeBase.h"

ACaveSpectatorPawn::ACaveSpectatorPawn(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    // dont replicate, pls
    SetReplicates(false);

    // no tick, pls
    SetActorTickEnabled(false);

    // we need begin play to setup stuff
    bAllowTickBeforeBeginPlay = false;

    // disable collision for now
    static FName NoCollision(TEXT("NoCollision"));
    GetCollisionComponent()->SetCollisionProfileName(NoCollision);
    GetCollisionComponent()->SetGenerateOverlapEvents(false);

    // just to make sure! there are no components
    DisableComponentsSimulatePhysics();

    // no input needed
    AutoPossessPlayer = EAutoReceiveInput::Disabled;
}

void ACaveSpectatorPawn::BeginPlay()
{
    Super::BeginPlay();

    // TODO: we dont need the default window unless we are on the server. The default window should have the same features as MultiViewportCameraActors Slate Window.
    if (GEngine)
    {
        auto *MyGameSettings = GEngine->GetGameUserSettings();
        MyGameSettings->SetFullscreenMode(EWindowMode::Windowed);

        if (GetWorld()->IsServer())
        {
            MyGameSettings->SetScreenResolution({640, 480});
        }
        else
        {
            MyGameSettings->SetScreenResolution({640, 480});
        }

        MyGameSettings->SetFrameRateLimit(60);
        MyGameSettings->SetWindowPosition(0, 0);
        MyGameSettings->ApplySettings(true);
        MyGameSettings->SaveSettings();
    }

    // Do we have a cave head yet? (server)
    // If no server to connect exists, the clients will spawn there own "cave head".
    // This will allow moving around etc. but will noch work in the actual cave.
    ACaveHeadCharacter *CaveHeadCharacter = nullptr;
    for (TActorIterator<ACaveHeadCharacter> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        CaveHeadCharacter = *ActorItr;
        break;
    }

    // only do this on server if there are no other clients connected
    if (GetWorld()->IsServer() && CaveHeadCharacter == nullptr)
    {
        UE_LOG(LogCave, Display,
               TEXT("I'm the boss (server). Spawning cave head character and destroying the spectator pawn."));

        // spawn the cave head character
        FActorSpawnParameters SpawnInfo;
        CaveHeadCharacter = GetWorld()->SpawnActor<ACaveHeadCharacter>(FVector(0, 0, 0), FRotator(0, 0, 0), SpawnInfo);

        // enable controlls for the head
        CaveHeadCharacter->PossessedBy(GetWorld()->GetFirstPlayerController());

        // despawn the servers spectator
        Destroy();

        return;
    }

    // disable all user inputs on clients
    DisableInput(nullptr);

    // we dont need movement for the spectators, they will be controlled by servers cave head
    GetMovementComponent()->Deactivate();
}
