    // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "SlateBasics.h"
#include "Runtime/UMG/Public/UMG.h"
#include "TextBoxWidget.generated.h"

// Just a widget to display text on the slate windows of the MultiViewportCameraActor instances
UCLASS()
class  UTextBoxWidget : public UUserWidget {
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    virtual TSharedRef<SWidget> RebuildWidget() override;

    UPROPERTY()
    UTextBlock* TextBlock;

    UPROPERTY(EditAnywhere)
    FString Text = FString("");

    UPROPERTY(EditAnywhere)
    FVector2D ViewportOffset = FVector2D(0, 0);

    UPROPERTY(EditAnywhere)
    FVector2D ViewportSize = FVector2D(1920, 1080);
};
