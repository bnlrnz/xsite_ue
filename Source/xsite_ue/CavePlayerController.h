// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CavePlayerController.generated.h"

/**
 *
 */
UCLASS()
class  ACavePlayerController : public APlayerController {
    GENERATED_BODY()

public:
    ACavePlayerController();
    // virtual void SetControlRotation(const FRotator& NewRotation) override;
};
