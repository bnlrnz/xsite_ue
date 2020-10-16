// Fill out your copyright notice in the Description page of Project Settings.

#include "CaveHeadCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine.h"
#include "MultiViewportCameraActor.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameModeBase.h"
#include "CaveGameModeBase.h"

static TAutoConsoleVariable<int> HeadTrackerID(
    TEXT("cave.CaveHeadCharacter.TrackerID"),
    1,
    TEXT(""),
    ECVF_Scalability | ECVF_RenderThreadSafe);

// Sets default values
ACaveHeadCharacter::ACaveHeadCharacter()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    SetReplicates(true);

    // Since we replicate by net nulticast, this is not needed
    SetReplicateMovement(false);

    // the defaults may be better?
    // needs investigation, 100 is default
    NetUpdateFrequency = 100.0f;

    // Replicate the pitch too
    bUseControllerRotationPitch = true;
    bUseControllerRotationYaw = true;
    bUseControllerRotationRoll = false;

    // Take control of the default player
    AutoPossessPlayer = EAutoReceiveInput::Player0;

    GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);

    // Since we replicate by net nulticast, this is not needed
    // Smoothing mode for simulated proxies in network game.
    // Disabled    -->  No smoothing, only change position as network position updates are received.
    // Linear      -->  Linear interpolation from source to target.
    // Exponential -->  Faster as you are further from target.
    // Replay      -->  Special linear interpolation designed specifically for replays.

    // Linear seems to work best for us (no, it's not, nothing works :()
    //GetCharacterMovement()->NetworkSmoothingMode = ENetworkSmoothingMode::Linear;

    // just to make sure! there are no components
    DisableComponentsSimulatePhysics();
}

void ACaveHeadCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ACaveHeadCharacter, bHeadtrackingEnabled);
    DOREPLIFETIME(ACaveHeadCharacter, HeadOrigin);
    DOREPLIFETIME(ACaveHeadCharacter, PlayerStartLocation);
    DOREPLIFETIME(ACaveHeadCharacter, PlayerStartRotation);
}

// Called when the game starts or when spawned
void ACaveHeadCharacter::BeginPlay()
{
    Super::BeginPlay();
    ToggleGhost();

    if (!GetWorld()->IsServer())
        return;

    auto CaveGameInstance = Cast<UCaveGameInstance>(GetWorld()->GetGameInstance());

    if (!IsValid(CaveGameInstance))
    {
        UE_LOG(LogCave, Error, TEXT("[ACaveHeadCharacter::BeginPlay] Could not obtain CaveGameInstance from GameInstance. Make shure your Game Instance is set to CaveGameInstance in the project settings."));
        return;
    }

    auto PlayerController = GetWorld()->GetFirstPlayerController();

    auto PlayerStart = GetWorld()->GetAuthGameMode()->K2_FindPlayerStart(PlayerController);

    this->PlayerStartLocation = PlayerStart->GetActorLocation();
    this->PlayerStartRotation = PlayerStart->GetActorRotation();

    // make the player spawn at player start and look in the correct direction
    PlayerController->SetControlRotation(this->PlayerStartRotation);
    SetActorLocationAndRotation(this->PlayerStartLocation, this->PlayerStartRotation);

    auto CaveController = CaveGameInstance->GetCaveController();

    if (!IsValid(CaveController))
    {
        UE_LOG(LogCave, Error, TEXT("[ACaveHeadCharacter::BeginPlay] Could not obtain CaveController from GameInstance. Make shure there is an instance of CaveController (e.g. Blueprint) in your scene and set up."));
        return;
    }

    HeadOrigin = CaveController->EyeOrigin;

    auto vrpnControllerActor = CaveGameInstance->GetVRPNControllerActor("DTrack");

    if (!IsValid(vrpnControllerActor))
    {
        UE_LOG(LogCave, Error, TEXT("[ACaveHeadCharacter::BeginPlay] Could not obtain VrpnControllerActor '%s' from GameInstance. Skipping..."), TEXT("DTRACK"));
        return;
    }
    
    auto vrpnController = vrpnControllerActor->GetController();

    if (vrpnController)
    {
        UE_LOG(LogCave, Warning, TEXT("[ACaveHeadCharacter::BeginPlay] Could not obtain VrpnController from CaveController. Ignore this, if you don't use VRPN."));
    }
    else
    {
        vrpnController->AddTrackerChangedCallback(
            [this, vrpnController](int32 sensor, VRPNController::TrackerData trackerData) {
                if (sensor != 0)
                    return;

                float xOffset = -HeadOrigin.X;
                float yOffset = -HeadOrigin.Y;
                float zOffset = -HeadOrigin.Z;

                this->HeadOrigin = FVector(
                    (float)(trackerData.Pos[1] + xOffset) * 100.f,
                    (float)(trackerData.Pos[0] + yOffset) * 100.f,
                    (float)(trackerData.Pos[2] + zOffset) * 100.f);

                this->HeadOrigin = this->GetController()->GetControlRotation().RotateVector(this->HeadOrigin);
            });
    }
}

void ACaveHeadCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (HasAuthority())
    {
        Multicast_UpdateLocationAndRotation(GetActorLocation(), GetActorRotation());
    }
}

FVector ACaveHeadCharacter::GetRealHeadLocation()
{
    if (bHeadtrackingEnabled)
    {
#if WITH_EDITOR
        //UE_LOG(LogCave, Warning, TEXT("GetRealHeadLocation (Editor): %s"), *this->GetActorLocation().ToString());
        return this->GetActorLocation() - this->PlayerStartLocation;
#else
        //UE_LOG(LogCave, Warning, TEXT("GetRealHeadLocation (Game): %s"), *this->HeadOrigin.ToString());
        return this->HeadOrigin;
#endif
    }
    return this->HeadOrigin;
}

// Called to bind functionality to input
void ACaveHeadCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
    // only allow input on master pawn
    if (GetWorld()->IsServer())
    {
        Super::SetupPlayerInputComponent(PlayerInputComponent);

        PlayerInputComponent->BindAction("ToggleHeadtracking", IE_Pressed, this,
                                         &ACaveHeadCharacter::ToggleHeadtracking);
        PlayerInputComponent->BindAction("ToggleGhost", IE_Pressed, this, &ACaveHeadCharacter::ToggleGhost);
        PlayerInputComponent->BindAction("ResetHead", IE_Pressed, this, &ACaveHeadCharacter::ResetHead);

        // set up gameplay key bindings
        PlayerInputComponent->BindAction("Slow", IE_Pressed, this, &ACaveHeadCharacter::SlowEnabled);
        PlayerInputComponent->BindAction("Slow", IE_Released, this, &ACaveHeadCharacter::SlowDisabled);

        PlayerInputComponent->BindAxis("MoveUp", this, &ACaveHeadCharacter::MoveUp);
        PlayerInputComponent->BindAxis("MoveForward", this, &ACaveHeadCharacter::MoveForward);
        PlayerInputComponent->BindAxis("StrafeRight", this, &ACaveHeadCharacter::StrafeRight);

        PlayerInputComponent->BindAxis("Turn", this, &ACaveHeadCharacter::AddControllerYawInput);
        PlayerInputComponent->BindAxis("LookUp", this, &ACaveHeadCharacter::AddControllerPitchInput);
    }

    PlayerInputComponent->BindAction("ExitGame", IE_Pressed, this, &ACaveHeadCharacter::ExitGame);
}

void ACaveHeadCharacter::ResetHead()
{
    auto CaveGameInstance = Cast<UCaveGameInstance>(GetWorld()->GetGameInstance());

    if (!IsValid(CaveGameInstance))
    {
        UE_LOG(LogCave, Error, TEXT("[ACaveHeadCharacter::ResetHead] Could not obtain CaveGameInstance from GameInstance. Make shure your Game Instance is set to CaveGameInstance in the project settings."));
        return;
    }

    auto CaveController = CaveGameInstance->GetCaveController();

    if (!IsValid(CaveController))
    {
        UE_LOG(LogCave, Error, TEXT("[ACaveHeadCharacter::ResetHead] Could not obtain CaveController from GameInstance. Make shure there is an instance of CaveController (e.g. Blueprint) in your scene and set up."));
        return;
    }

    auto *PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

    if (PlayerController)
    {
        PlayerController->SetControlRotation(this->PlayerStartRotation);
        SetActorLocationAndRotation(this->PlayerStartLocation, this->PlayerStartRotation);
    }
}

void ACaveHeadCharacter::Multicast_UpdateLocationAndRotation_Implementation(FVector Loc, FRotator Rot)
{
    NetLoc = Loc;
    NetRot = Rot;
}

void ACaveHeadCharacter::Multicast_ExitGame_Implementation()
{
    auto *PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

    if (PlayerController)
    {
        PlayerController->ConsoleCommand("quit");
    }
}

void ACaveHeadCharacter::Multicast_ExecuteCommand_Implementation(const FString &Command)
{
    auto *PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

    if (PlayerController)
    {
        PlayerController->ConsoleCommand(Command);
    }
}

void ACaveHeadCharacter::Multicast_ToggleGhost_Implementation()
{
    auto *CharacterMovementPtr = GetCharacterMovement();

    if (CharacterMovementPtr == nullptr)
    {
        return;
    }

    if (bIsGhost)
    {
        UE_LOG(LogCave, Display, TEXT("Disabled ghost mode."));
        bIsGhost = false;

        // normal mode
        CharacterMovementPtr->StopMovementImmediately();
        CharacterMovementPtr->SetMovementMode(EMovementMode::MOVE_Walking);
        CharacterMovementPtr->bCheatFlying = false;
        SetActorEnableCollision(true);
    }
    else
    {
        UE_LOG(LogCave, Display, TEXT("Enabled ghost mode."));
        bIsGhost = true;

        // ghost mode
        CharacterMovementPtr->StopMovementImmediately();
        CharacterMovementPtr->SetMovementMode(EMovementMode::MOVE_Flying);
        CharacterMovementPtr->bCheatFlying = true;
        SetActorEnableCollision(false);
    }
}

void ACaveHeadCharacter::ToggleGhost() { Multicast_ToggleGhost(); }

void ACaveHeadCharacter::ExitGame()
{
    // dont do this in editor, otherwise the editor will get killed too
    if (GetWorld()->IsPlayInEditor() || GetWorld()->IsPlayInPreview())
    {
        UE_LOG(LogCave, Warning, TEXT("Skipping the shutdown, I'm the editor."));
        return;
    }

    // try shutdown clients
    Multicast_ExitGame();

    UE_LOG(LogCave, Error, TEXT("ESC pressed by user. Trying to tear down game instance..."));
}

void ACaveHeadCharacter::ToggleHeadtracking()
{
    // toggle headtracking
    bHeadtrackingEnabled = !bHeadtrackingEnabled;
    UE_LOG(LogCave, Warning, TEXT("Headtracking: %s"), bHeadtrackingEnabled ? TEXT("true") : TEXT("false"));
}

void ACaveHeadCharacter::MoveForward(float Value)
{
    //if a screen was selected dont do normal movement
    //    if (SelectedRuntimeScreen) {
    //        const FVector CurrentLocation = SelectedRuntimeScreen->ScreenProceduralMeshComponent->GetRelativeLocation()
    //                                        + FVector(0.0f, 0.0f, Value * SpeedFactor);
    //        SelectedRuntimeScreen->ScreenProceduralMeshComponent->GetRelativeLocation() = CurrentLocation;
    //        return;
    //    }

    if ((Controller != nullptr) && (Value != 0.0f))
    {
        // find out which way is forward
        FRotator Rotation = Controller->GetControlRotation();
        // Limit pitch when walking or falling
        if (GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling())
        {
            Rotation.Pitch = 0.0f;
        }
        // add movement in that direction
        const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);
        AddMovementInput(Direction, Value * SpeedFactor);
    }
}

void ACaveHeadCharacter::StrafeRight(float Value)
{
    //if a screen was selected dont do normal movement
    //    if (SelectedRuntimeScreen) {
    //        const FVector CurrentLocation = SelectedRuntimeScreen->ScreenProceduralMeshComponent->GetRelativeLocation()
    //                                        + FVector(0.0f, Value * SpeedFactor, 0.0f);
    //        SelectedRuntimeScreen->ScreenProceduralMeshComponent->GetRelativeLocation() = CurrentLocation;
    //        return;
    //    }

    if ((Controller != nullptr) && (Value != 0.0f))
    {
        // find out which way is right
        const FRotator Rotation = Controller->GetControlRotation();
        const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);
        // add movement in that direction
        AddMovementInput(Direction, Value * SpeedFactor);
    }
}

void ACaveHeadCharacter::MoveUp(float Value)
{
    //if a screen was selected dont do normal movement
    //    if (SelectedRuntimeScreen) {
    //        const FVector CurrentLocation = SelectedRuntimeScreen->ScreenProceduralMeshComponent->GetRelativeLocation()
    //                                        + FVector(Value * SpeedFactor, 0.0, 0.0f);
    //        SelectedRuntimeScreen->ScreenProceduralMeshComponent->GetRelativeLocation() = CurrentLocation;
    //        return;
    //    }

    if ((Controller != nullptr) && (Value != 0.0f))
    {
        // find out which way is right
        const FRotator Rotation = Controller->GetControlRotation();
        const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Z);
        // add movement in that direction
        AddMovementInput(Direction, Value * SpeedFactor);
    }
}

void ACaveHeadCharacter::SlowEnabled()
{
    SpeedFactor = 0.1f;
}

void ACaveHeadCharacter::SlowDisabled()
{
    SpeedFactor = 1.0f;
}

void ACaveHeadCharacter::Multicast_IdentifyScreen_Implementation()
{
    if (GetWorld() == nullptr)
        return;

    for (TActorIterator<AMultiViewportCameraActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        auto *CameraActor = *ActorItr;
        if (CameraActor->bEnabledScreen)
        {
            CameraActor->IdentifyScreen();
        }
    }
}

void ACaveHeadCharacter::Multicast_IdentifyScreenClear_Implementation()
{
    if (GetWorld() == nullptr)
        return;

    for (TActorIterator<AMultiViewportCameraActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        auto *CameraActor = *ActorItr;
        if (CameraActor->bEnabledScreen)
        {
            CameraActor->IdentifyScreenClear();
        }
    }
}

void ACaveHeadCharacter::Multicast_Warping_Implementation(const FString &hostname, bool enabled)
{
    if (GetWorld() == nullptr)
        return;

    auto CaveGameInstance = (UCaveGameInstance *)GetWorld()->GetGameInstance();

    // is it me you're looking for?
    if (!CaveGameInstance->GetComputerName().Equals(hostname))
    {
        // UE_LOG(LogCave, Warning, TEXT("Was looking for %s, found %s instead :( "), *hostname, *CaveGameInstance->GetComputerName());
        return;
    }

    for (TActorIterator<AMultiViewportCameraActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        auto *CameraActor = *ActorItr;
        CameraActor->bUseWarping = enabled;
    }
}

void ACaveHeadCharacter::Multicast_Blending_Implementation(const FString &hostname, bool enabled)
{
    if (GetWorld() == nullptr)
        return;

    auto CaveGameInstance = (UCaveGameInstance *)GetWorld()->GetGameInstance();

    // is it me you're looking for?
    if (!CaveGameInstance->GetComputerName().Equals(hostname))
    {
        // UE_LOG(LogCave, Warning, TEXT("Was looking for %s, found %s instead :( "), *hostname, *CaveGameInstance->GetComputerName());
        return;
    }

    for (TActorIterator<AMultiViewportCameraActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        auto *CameraActor = *ActorItr;
        CameraActor->bUseBlending = enabled;
    }
}

void ACaveHeadCharacter::Multicast_WarpingAll_Implementation(bool enabled)
{
    if (GetWorld() == nullptr)
        return;

    for (TActorIterator<AMultiViewportCameraActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        auto *CameraActor = *ActorItr;
        CameraActor->bUseWarping = enabled;
    }
}

void ACaveHeadCharacter::Multicast_BlendingAll_Implementation(bool enabled)
{
    if (GetWorld() == nullptr)
        return;

    for (TActorIterator<AMultiViewportCameraActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        auto *CameraActor = *ActorItr;
        CameraActor->bUseBlending = enabled;
    }
}

void ACaveHeadCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}
