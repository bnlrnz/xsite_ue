// Fill out your copyright notice in the Description page of Project Settings.

#include "OffAxisLocalPlayer.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine.h"

// These "illegal" pointers to private members were necessary before
// the SceneView::UpdateProjectionMatrix(FMatrix ...) method was introduced
// static void UpdateProjectionMatrix(FSceneView *View) {
//     // this is a bit hacky but access is only private for these matrices :(

//     // TODO: fix the temporal AA matrix too

//     // OLD Version
//     auto *pProjectionMatrix = (FMatrix *) (&View->ViewMatrices.GetProjectionMatrix());
//     *pProjectionMatrix = AdjustProjectionMatrixForRHI(View->ProjectionMatrixUnadjustedForRHI);

//     auto *pViewProjectionMatrix = (FMatrix *) (&View->ViewMatrices.GetViewProjectionMatrix());
//     *pViewProjectionMatrix = View->ViewMatrices.GetViewMatrix() * View->ViewMatrices.GetProjectionMatrix();

//     auto *pInvViewProjectionMatrix = (FMatrix *) &View->ViewMatrices.GetInvViewProjectionMatrix();
//     *pInvViewProjectionMatrix = View->ViewMatrices.GetViewProjectionMatrix().Inverse();

//     FMatrix TranslatedViewMatrix =
//             FTranslationMatrix(-View->ViewMatrices.GetPreViewTranslation()) * View->ViewMatrices.GetViewMatrix();

//     auto *pTranslatedViewProjectionMatrix = (FMatrix *) (&View->ViewMatrices.GetTranslatedViewProjectionMatrix());
//     *pTranslatedViewProjectionMatrix = TranslatedViewMatrix * View->ViewMatrices.GetProjectionMatrix();

//     auto *pInvTranslatedViewProjectionMatrixx = (FMatrix *) (&View->ViewMatrices.GetInvTranslatedViewProjectionMatrix());
//     *pInvTranslatedViewProjectionMatrixx = View->ViewMatrices.GetTranslatedViewProjectionMatrix().Inverse();

//     View->ShadowViewMatrices = View->ViewMatrices;

//     GetViewFrustumBounds(View->ViewFrustum, View->ViewMatrices.GetViewProjectionMatrix(), false);
// }

FSceneView *
UOffAxisLocalPlayer::CalcSceneView(FSceneViewFamily *ViewFamily, FVector &OutViewLocation, FRotator &OutViewRotation,
                                   FViewport *Viewport, FViewElementDrawer *ViewDrawer, EStereoscopicPass StereoPass)
{
    // calculate the default projection matrix
    auto View = ULocalPlayer::CalcSceneView(ViewFamily, OutViewLocation, OutViewRotation, Viewport, ViewDrawer,
                                            StereoPass);

    if (!View)
    {
        return View;
    }

    // get the actor, he contains the offset and rotation, or screen corner coordinates
    AMultiViewportCameraActor* ViewActor = const_cast<AMultiViewportCameraActor*>(Cast<AMultiViewportCameraActor>(View->ViewActor));

    // check if our current actor is not null and is really the multi viewport camera actor
    if (!ViewActor || !ViewActor->IsA(AMultiViewportCameraActor::StaticClass()))
    {
        return View;
    }

    auto *CaveGameInstance = Cast<UCaveGameInstance>(GetWorld()->GetGameInstance());

    // calculate own off-axis projection matrix based on screen meshs corners and the current eye point
    // we could cache the matrix and eye point and check if we really have to recalculate this matrix (only if eye point/head changed)
    if (ViewActor->bShouldCalculateViewMatrix ||
        CaveGameInstance->GetCaveHeadCharacter()->bHeadtrackingEnabled)
    {

        //adds or overwrites the view matrix for a specific screen
        CachedViewMatrices.Add(ViewActor->WindowTitle.ToString(), GenerateOffAxisMatrix(ViewActor));

        // mark as calculated
        ViewActor->bShouldCalculateViewMatrix = false;
    }

    // override the view matrix and update all the needed projections
    View->ProjectionMatrixUnadjustedForRHI = CachedViewMatrices[ViewActor->WindowTitle.ToString()];
    View->UpdateProjectionMatrix(CachedViewMatrices[ViewActor->WindowTitle.ToString()]);

    return View;
}

FMatrix UOffAxisLocalPlayer::GenerateOffAxisMatrix(const AMultiViewportCameraActor *Actor)
{
    // near and far clipping plane
    float n = GNearClippingPlane;
    float f = 30000.0f;

    // get some screen space coordinates
    //
    //   pc_________
    //    |        |
    //    | Screen |
    //    |________|
    //   pa        pb
    //
    //
    // x(0,0)                 x <-pe

    if (Actor == nullptr)
    {
        UE_LOG(LogCave, Error, TEXT("[GenerateOffAxisMatrix] Actor not available :("));
        exit(0);
    }

    // lower left -> pa
    const FVector pa(Actor->ScreenBottomLeft);

    // lower right -> pb
    const FVector pb(Actor->ScreenBottomRight);

    // upper left -> pc
    const FVector pc(Actor->ScreenTopLeft);

    // eye position
    FVector pe(0, 0, 0);

    auto *CaveGameInstance = Cast<UCaveGameInstance>(GetWorld()->GetGameInstance());

    // this should be cave head character location
    if (CaveGameInstance != nullptr && CaveGameInstance->GetCaveHeadCharacter() != nullptr)
    {
        pe = CaveGameInstance->GetCaveHeadCharacter()->GetRealHeadLocation();
    }

    // create orthonormal basis of the screen
    //
    //     pc___________________
    //      |        vu        |
    //      |        |         |
    //      |        |___vr    |
    //      |       /          |
    //      |      vn          |
    //      |__________________|
    //     pa                  pb
    //
    FVector vr, vu, vn;

    vr = (pb - pa);
    vr.Normalize();

    vu = (pc - pa);
    vu.Normalize();

    vn = -FVector::CrossProduct(vr, vu);
    vn.Normalize();

    // create screen corner vectors
    FVector va, vb, vc;
    va = pa - pe;
    vb = pb - pe;
    vc = pc - pe;

    // get the distance from the eye to the screen space origin
    // -> dot product of the screen normal vn with any screen vector (va, vb, vc)
    float d = -FVector::DotProduct(vn, va);

    // get the extents of the (perpendicular for now) perspective projection
    //
    //     __l__|_r_
    //     ________
    //    |        |    |
    //    |     x  |    |__t
    //    |________|    |  b
    //
    // usually, in a classic perspective projection, l == r and t == b
    // this is (usually) not the case for us
    //
    float l, r, t, b;
    float scale = n / d; // scaling by distance from eye to near clipping plane divided by distance from

    l = FVector::DotProduct(vr, va) * scale;
    r = FVector::DotProduct(vr, vb) * scale;
    t = FVector::DotProduct(vu, vc) * scale;
    b = FVector::DotProduct(vu, va) * scale;

    // create the (perpendicular for now) perspective projection matrix
    FMatrix OffAxisProjectionMatrix;

    // fill with zeros and ones at the diagonal
    OffAxisProjectionMatrix.SetIdentity();

    // fill in the extents
    OffAxisProjectionMatrix.M[0][0] = (2 * n) / (r - l);
    OffAxisProjectionMatrix.M[1][1] = (2 * n) / (t - b);
    OffAxisProjectionMatrix.M[2][0] = (r + l) / (r - l);
    OffAxisProjectionMatrix.M[2][1] = (t + b) / (t - b);
    OffAxisProjectionMatrix.M[2][2] = 0.0; /*-(f + n) / (f - n);*/
    OffAxisProjectionMatrix.M[2][3] = 1.0f;
    OffAxisProjectionMatrix.M[3][2] = n; /*-(2 * f * n) / (f - n);*/
    OffAxisProjectionMatrix.M[3][3] = 0.0f;

    // now we have a "default" perspective projection matrix
    // we need to rotate the screen plane out of the XY plane (which would be the default case)
    // we also need to position the screen plane correctly relative to the eye/view point aka head tracking position

    //////////////
    // Rotation
    //////////////

    //FRotator Rotator = Actor->ScreenRotation;
    FRotator Rotator = (-vn).Rotation();

    FRotationMatrix RotationMatrix = FRotationMatrix(FRotator(Rotator.Yaw, Rotator.Roll, -Rotator.Pitch));
    //FRotationMatrix RotationMatrix = FRotationMatrix(FRotator(Rotator.Pitch, Rotator.Yaw, Rotator.Roll));

    OffAxisProjectionMatrix = RotationMatrix * OffAxisProjectionMatrix;

    //////////////
    // Translation -> why does this work? needs further investigation...
    //////////////

    // depending on the screen orientation this could change
    // the "floor" got the screen in the XY plane
    // the "front" got the screen in the YZ plane etc.

    FVector ScreenCenter = (pb + pc) / 2.0;

    // get the static screen offset
    FVector ScreenUp = -4 * vu * (ScreenCenter - pe) / Actor->ScreenSize[1];
    FVector ScreenRight = -4 * vr * (ScreenCenter - pe) / Actor->ScreenSize[0];
    // | pipe is dot product
    FVector ScreenOffset = FVector(ScreenRight | vr.GetAbs(), ScreenUp | vu.GetAbs(), 0);

    // apply the offset
    FTranslationMatrix Translation = FTranslationMatrix(ScreenOffset);
    OffAxisProjectionMatrix = OffAxisProjectionMatrix * Translation;

    //DEBUG
    // UE_LOG(LogCave, Warning, TEXT("=============\nGenerateOffAxisMatrix (%s) @ %s\nEye: %s\n=====\nPa/BL: %s\nPb/BR: %s\nPc/TL: %s\n===============\nScreenUp: %s\nScreenRight: %s\n===============\nScreenRotation(vn): %s\nScreenRotation(normal): %s\nScreenOffset: %s\nScreenSize: %s\n==========\n"),
    //     *Actor->WindowTitle.ToString(),
    //     *ScreenCenter.ToString(),
    //     *pe.ToString(), *pa.ToString(), *pb.ToString(), *pc.ToString(),
    //     *ScreenUp.ToString(), *ScreenRight.ToString(),
    //     *Rotator.ToString(), *Actor->ScreenRotation.ToString(),
    //     *ScreenOffset.ToString(), *Actor->ScreenSize.ToString()
    // );

    return OffAxisProjectionMatrix;
}