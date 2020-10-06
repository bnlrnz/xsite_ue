// Fill out your copyright notice in the Description page of Project Settings.

#include "PNGImageLoader.h"
#include "Modules/ModuleManager.h"
#include "Misc/FileHelper.h"

bool PNGImageLoader::FromFile(const FString &FilePathAndName, UTexture2D *&OutTexture, int32 &OutWidth, int32 &OutHeight)
{

    TArray<uint8> ImageData;

    auto &ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));

    TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

    //file available
    if (!FFileHelper::LoadFileToArray(ImageData, *FilePathAndName))
    {
        return false;
    }

    //image wrapper there? able to operate image?
    if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(ImageData.GetData(), ImageData.Num()))
    {
        TArray<uint8> UncompressedBGRA;

        if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA))
        {
            OutTexture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);

            if (!OutTexture)
            {
                return false;
            }

            OutTexture->AddToRoot();

            OutWidth = ImageWrapper->GetWidth();
            OutHeight = ImageWrapper->GetHeight();

#if WITH_EDITORONLY_DATA
            OutTexture->MipGenSettings = TMGS_NoMipmaps;
#endif

            void *TextureData = OutTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
            FMemory::Memcpy(TextureData, UncompressedBGRA.GetData(), UncompressedBGRA.Num());
            OutTexture->PlatformData->Mips[0].BulkData.Unlock();

            OutTexture->UpdateResource();

            return true;
        }
    }

    return false;
}
