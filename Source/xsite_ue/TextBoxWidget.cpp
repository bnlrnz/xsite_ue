// Fill out your copyright notice in the Description page of Project Settings.

#include "TextBoxWidget.h"

void UTextBoxWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind delegates here.
}

TSharedRef<SWidget> UTextBoxWidget::RebuildWidget()
{
    UPanelWidget *RootWidget = Cast<UPanelWidget>(GetRootWidget());

    if (!RootWidget)
    {
        // construct the root widget if it does not exist
        // root widgets are UCanvasPanels by default on UMG
        RootWidget = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootWidget"));

        // we need the slot. all widget layout properties are handled in the widget's slot
        UCanvasPanelSlot *RootWidgetSlot = Cast<UCanvasPanelSlot>(RootWidget->Slot);

        if (RootWidgetSlot)
        {
            // fullscreen widget!
            RootWidgetSlot->SetAnchors(FAnchors(0, 0, 1, 1));
            RootWidgetSlot->SetOffsets(FMargin(0, 0, 0, 0));
        }

        // set it as root
        WidgetTree->RootWidget = RootWidget;
    }

    TSharedRef<SWidget> Widget = Super::RebuildWidget();

    if (RootWidget && WidgetTree)
    {
        TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TextBlock"));
        TextBlock->SetText(FText::FromString(Text));
        TextBlock->Font.Size = int32(0.05 * ViewportSize.Y);
        RootWidget->AddChild(TextBlock);
        UCanvasPanelSlot *TextBlockSlot = Cast<UCanvasPanelSlot>(TextBlock->Slot);
        if (TextBlockSlot)
        {
            TextBlockSlot->SetPosition(ViewportOffset);
            TextBlockSlot->SetAutoSize(true);
        }
    }

    return Widget;
}
