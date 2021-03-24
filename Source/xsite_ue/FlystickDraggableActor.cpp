// Fill out your copyright notice in the Description page of Project Settings.

#include "FlystickDraggableActor.h"
#include "CaveGameInstance.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

static TAutoConsoleVariable<float> ThrowMultiplicator(
    TEXT("cave.FlystickDraggableActor.ThrowMultiplicator"),
    1000.0f,
    TEXT(""),
    ECVF_Scalability | ECVF_RenderThreadSafe);

// Sets default values
AFlystickDraggableActor::AFlystickDraggableActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // init our mesh
    this->Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

    this->RootComponent = this->Mesh;
    this->Mesh->SetIsReplicated(true);
    bReplicates = true;
}

// Called when the game starts or when spawned
void AFlystickDraggableActor::BeginPlay()
{
    Super::BeginPlay();

    if (GetWorld()->IsServer())
    {
        auto *PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        this->EnableInput(PlayerController);

        if (IsValid(this->Mesh))
        {
            this->Mesh->SetSimulatePhysics(true);
            this->Mesh->SetEnableGravity(true);
        }
    }
    else
    {
        // clients mesh gets updated by replicated server data
        if (IsValid(this->Mesh))
        {
            this->Mesh->SetSimulatePhysics(false);
            this->Mesh->SetEnableGravity(false);
            this->Mesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
            this->Mesh->PutRigidBodyToSleep();
        }
    }
}

// Called every frame
void AFlystickDraggableActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // get the cave head
    if (this->CaveHeadCharacter == nullptr)
    {
        auto *CaveGameInstance = (UCaveGameInstance *)GetWorld()->GetGameInstance();
        this->CaveHeadCharacter = CaveGameInstance->GetCaveHeadCharacter();
    }

    // only on server
    if (!GetWorld()->IsServer())
        return;

    // get the tossing stuff
    this->TossRecognitonTimeAccu += DeltaTime;
    if (this->TossRecognitonTimeAccu > this->TossRecognitionIntervall)
    {
        this->TossStart = this->GetActorLocation();
        this->TossRecognitonTimeAccu = 0.0f;
    }

    // check, if we got pointed at
    if (this->Flystick == nullptr)
        return;

    if (!IsValid(this->Flystick))
    {
        UE_LOG(LogCave, Warning, TEXT("Flystick invalid on %s"), *this->GetName());
        return;
    }

    // check, if we are attached to a flystick
    if (!this->bAttachedToFlystick)
        return;

    //    UE_LOG(LogCave, Warning, TEXT("Attached Actor: %s me: %s"), *this->Flystick->AttachedActorName, *this->GetName());

    if (!this->Flystick->AttachedActorName.Equals(this->GetName()))
        return;

    //UE_LOG(LogCave, Warning, TEXT("Dragging: %s"), *this->GetName());

    this->RootComponent->SetWorldLocation(this->Flystick->End * (1.01f) + this->CaveHeadCharacter->NetLoc,
                                          false, nullptr, ETeleportType::ResetPhysics);
}

void AFlystickDraggableActor::SelectedCallback_Implementation(AVRPNFlystickActor *AFlystick, FVector HitLocation)
{
    //UE_LOG(LogCave, Warning, TEXT("Selected: %s"), *this->GetName());
    this->Flystick = AFlystick;
}

void AFlystickDraggableActor::DeselectedCallback_Implementation(AVRPNFlystickActor *AFlystick)
{
    //UE_LOG(LogCave, Warning, TEXT("Deselected: %s"), *this->GetName());

    //only deselect if not attached
    if (!this->bAttachedToFlystick)
        this->Flystick = nullptr;
}

void AFlystickDraggableActor::ClickedAtCallback_Implementation(AVRPNFlystickActor *AFlystick, FVector HitLocation, ButtonState state)
{
    //UE_LOG(LogCave, Warning, TEXT("Clicked: %s == %s : - %s"), *this->GetName(), *AFlystick->AttachedActorName, state == ButtonState::Pressed ? TEXT("Pressed") : TEXT("Released"));

    this->Flystick = AFlystick;

    // toggle
    this->bAttachedToFlystick = state == ButtonState::Pressed;

    if (this->bAttachedToFlystick)
    {
        if (IsValid(this->Mesh))
        {
            this->Mesh->SetSimulatePhysics(false);
            this->Mesh->SetEnableGravity(false);
        }
    }
    else
    {
        if (IsValid(this->Mesh))
        {
            this->Mesh->SetSimulatePhysics(true);
            this->Mesh->SetEnableGravity(true);
            this->Mesh->AddForce((this->GetActorLocation() - this->TossStart) * ThrowMultiplicator.GetValueOnGameThread() * this->Mesh->GetMass());
        }
    }
}
