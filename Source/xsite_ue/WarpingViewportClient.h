// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"
#include "WarpingViewportClient.generated.h"

// UNUSED!

// TODO: Currently the warping is done in the fragment shader/as blendable post processing effect. We could also render our scene to an offscreen render buffer/texture and map this texture on a warped mesh.

/**
 * To tell the engine to use our custom class instead of the base engine version 
 * we have to add the following into the projects DefaultEngine.ini file.
 * [/Script/Engine.Engine]
 * GameViewportClientClassName=/Script/MultiOffAxisViewport.UWarpingViewportClient
 */
UCLASS()
class  UWarpingViewportClient : public UGameViewportClient
{
	GENERATED_BODY()
	
	UWarpingViewportClient(const FObjectInitializer& ObjectInitializer);

	/**
	 * Called after rendering the player views and HUDs to render menus, the console, etc.
	 * This is the last rendering call in the render loop
	 *
	 * @param Canvas        The canvas to use for rendering.
	 */
	virtual void PostRender( UCanvas* Canvas ) override;

private:
	UTextureRenderTarget2D* CreateRenderTarget2D(int32 Width = 1920, int32 Height = 1080);
};
