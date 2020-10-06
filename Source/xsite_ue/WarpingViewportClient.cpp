// Fill out your copyright notice in the Description page of Project Settings.


#include "WarpingViewportClient.h"
#include "Engine/Canvas.h"
#include "Engine/TextureRenderTarget2D.h"

UWarpingViewportClient::UWarpingViewportClient(const FObjectInitializer& ObjectInitializer)
: UGameViewportClient(ObjectInitializer){
    
}

void UWarpingViewportClient::PostRender(UCanvas* Canvas)
{
    UGameViewportClient::PostRender(Canvas);

    // Todo: Render to warped mesh here?

    // FLinearColor Color = FLinearColor::Black;
    // Color.A = 0.8;

    // Canvas->DrawColor = Color.ToFColor(true);
    
    // Canvas->DrawTile(Canvas->DefaultTexture, 0, 0, Canvas->ClipX, Canvas->ClipY, 0, 0, Canvas->DefaultTexture->GetSizeX(), Canvas->DefaultTexture->GetSizeY());
    
}

UTextureRenderTarget2D* UWarpingViewportClient::CreateRenderTarget2D(int32 Width, int32 Height){
    UTextureRenderTarget2D* RenderTarget2D = NewObject<UTextureRenderTarget2D>();

    if(!RenderTarget2D)
    {
        return nullptr;
    }

    RenderTarget2D->ClearColor = FLinearColor(255,15,200);
    RenderTarget2D->InitAutoFormat(Width, Height);
    return RenderTarget2D;
}