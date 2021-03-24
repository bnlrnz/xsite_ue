// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "Runtime/ImageWrapper/Public/IImageWrapper.h"
#include "Runtime/ImageWrapper/Public/IImageWrapperModule.h"

/**
 * Helper Class to load png images at runtime - this is used to load the blending masks
 */
class PNGImageLoader
{
public:
	static bool FromFile(const FString &FilePathAndName, UTexture2D *&OutTexture, int32 &OutWidth, int32 &OutHeight);

private:
	PNGImageLoader() = default;
	~PNGImageLoader() = default;
};
