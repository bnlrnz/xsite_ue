// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VRPNFlystickActor.h"

#include "FlystickManipulatable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UFlystickManipulatable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class  IFlystickManipulatable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SelectedCallback(AVRPNFlystickActor* Flystick, FVector HitLocation);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void DeselectedCallback(AVRPNFlystickActor* Flystick);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void ClickedAtCallback(AVRPNFlystickActor* Flystick, FVector HitLocation, ButtonState state);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void DragCallback(AVRPNFlystickActor* Flystick, FVector HitLocation);
};
