// Fill out your copyright notice in the Description page of Project Settings.

#include "CavePlayerController.h"
#include "CavePlayerCameraManager.h"

ACavePlayerController::ACavePlayerController()
{
    PlayerCameraManagerClass = ACavePlayerCameraManager::StaticClass();
}

//  void ACavePlayerController::SetControlRotation(const FRotator& NewRotation)
//  {
// 	if (!IsValidControlRotation(NewRotation))
// 	{
// 		logOrEnsureNanError(TEXT("AController::SetControlRotation attempted to apply NaN-containing or NaN-causing rotation! (%s)"), *NewRotation.ToString());
// 		return;
// 	}

// 	if (!ControlRotation.Equals(NewRotation, 1e-3f))
// 	{
// 		ControlRotation = NewRotation;

// 		if (RootComponent && RootComponent->IsUsingAbsoluteRotation())
// 		{
// 			RootComponent->SetWorldRotation(GetControlRotation());
// 		}
// 	}
// 	else
// 	{
// 		//UE_LOG(LogPlayerController, Log, TEXT("Skipping SetControlRotation %s for %s (Pawn %s)"), *NewRotation.ToString(), *GetNameSafe(this), *GetNameSafe(GetPawn()));
// 	}
//  }