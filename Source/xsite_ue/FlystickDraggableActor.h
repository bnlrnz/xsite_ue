// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlystickManipulatable.h"
#include "VRPNFlystickActor.h"
#include "CaveHeadCharacter.h"
#include "FlystickDraggableActor.generated.h"

UCLASS()
class AFlystickDraggableActor : public AActor, public IFlystickManipulatable
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AFlystickDraggableActor();

    // Static Mesh Comp, Set In BP Default Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = StaticMeshComponents)
    UStaticMeshComponent *Mesh;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void SelectedCallback(AVRPNFlystickActor *AFlystick, FVector HitLocation);
    virtual void SelectedCallback_Implementation(AVRPNFlystickActor *AFlystick, FVector HitLocation) override;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void DeselectedCallback(AVRPNFlystickActor *AFlystick);
    virtual void DeselectedCallback_Implementation(AVRPNFlystickActor *AFlystick) override;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void ClickedAtCallback(AVRPNFlystickActor *AFlystick, FVector HitLocation, ButtonState state);
    virtual void ClickedAtCallback_Implementation(AVRPNFlystickActor *AFlystick, FVector HitLocation, ButtonState state) override;

    UPROPERTY(BlueprintReadOnly)
    bool bAttachedToFlystick = false;

    UPROPERTY(BlueprintReadOnly)
    AVRPNFlystickActor *Flystick = nullptr;

private:
    UPROPERTY()
    ACaveHeadCharacter *CaveHeadCharacter = nullptr;

    FVector TossStart = FVector(0, 0, 0);
    float TossRecognitonTimeAccu = 0.0f;
    const float TossRecognitionIntervall = 0.01f;
};
