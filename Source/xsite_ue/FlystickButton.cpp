// Fill out your copyright notice in the Description page of Project Settings.

#include "FlystickButton.h"

// Sets default values
AFlystickButton::AFlystickButton()
{
    this->SetReplicates(true);

    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // init our mesh
    this->Widget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
    this->RootComponent = this->Widget;
    this->Widget->SetTwoSided(true);
}

// Called when the game starts or when spawned
void AFlystickButton::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void AFlystickButton::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (this->bRecentlyClicked)
    {
        this->BlendingTimeCurrently += DeltaTime;

        float alpha = this->BlendingTimeCurrently / this->BlendingTime;
        this->Widget->SetTintColorAndOpacity(FLinearColor(0.5 * alpha, 0.4 * alpha, 0.07 * alpha, 0.5));

        if (this->BlendingTimeCurrently >= this->BlendingTime)
        {
            this->bRecentlyClicked = false;
            this->BlendingTimeCurrently = 0.0f;

            if (this->bSelected)
            {
                this->Widget->SetTintColorAndOpacity(FLinearColor(0.5, 0.4, 0.07, 0.5));
            }
            else
            {
                this->Widget->SetTintColorAndOpacity(FLinearColor(1, 1, 1, 0.5));
            }
        }
    }
}

void AFlystickButton::SelectedCallback_Implementation(AVRPNFlystickActor *AFlystick, FVector HitLocation)
{
    // only if we got a new selection
    if (this->bRecentlyClicked || this->bSelected)
        return;
    Multicast_Selected();
}

void AFlystickButton::DeselectedCallback_Implementation(AVRPNFlystickActor *AFlystick)
{
    Multicast_Deselected();
}

void AFlystickButton::ClickedAtCallback_Implementation(AVRPNFlystickActor *AFlystick, FVector HitLocation, ButtonState state)
{
    // only "real" click
    if (state != ButtonState::Released)
        return;

    Multicast_Clicked();
}

void AFlystickButton::Multicast_Clicked_Implementation()
{
    this->bRecentlyClicked = true;
    this->BlendingTimeCurrently = 0.0f;
}

void AFlystickButton::Multicast_Selected_Implementation()
{
    this->Widget->SetTintColorAndOpacity(FLinearColor(0.5, 0.4, 0.07, 0.5));
}

void AFlystickButton::Multicast_Deselected_Implementation()
{
    this->Widget->SetTintColorAndOpacity(FLinearColor(1, 1, 1, 0.5));
}