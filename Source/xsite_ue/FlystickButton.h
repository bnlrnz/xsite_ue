// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/UMGStyle.h"
#include "Runtime/UMG/Public/Slate/SObjectWidget.h"
#include "Runtime/UMG/Public/IUMGModule.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/Components/WidgetComponent.h"
#include "CoreMinimal.h"
#include "FlystickManipulatable.h"
#include "GameFramework/Actor.h"
#include "FlystickButton.generated.h"

UCLASS()
class AFlystickButton : public AActor, public IFlystickManipulatable
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AFlystickButton();

    // Widget Comp, Set In BP Default Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WidgetComponents)
    UWidgetComponent *Widget;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void SelectedCallback(AVRPNFlystickActor *AFlystick, FVector HitLocation);
    virtual void SelectedCallback_Implementation(AVRPNFlystickActor *AFlystick, FVector HitLocation) override;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void DeselectedCallback(AVRPNFlystickActor *AFlystick);
    virtual void DeselectedCallback_Implementation(AVRPNFlystickActor *AFlystick) override;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void ClickedAtCallback(AVRPNFlystickActor *AFlystick, FVector HitLocation, ButtonState state);
    virtual void ClickedAtCallback_Implementation(AVRPNFlystickActor *AFlystick, FVector HitLocation, ButtonState state) override;

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_Selected();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_Deselected();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_Clicked();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

private:
    const float BlendingTime = 0.2f;
    float BlendingTimeCurrently = 0.0f;
    bool bRecentlyClicked = false;
    bool bSelected = false;
};
