// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "xsite_ue.h"
#include "ScreenRuntimeWarpMeshActor.generated.h"

// UNUSED!

// TODO: parameterize this with screen
// Spawn like: AScreenRuntimeWarpMeshActor* WarpMesh = GetWorld()->SpawnActor<AScreenRuntimeWarpMeshActor>(AScreenRuntimeWarpMeshActor::StaticClass(), FVector(0,0,0), FRotator(0,90,0));

// TODO: Currently the warping is done in the fragment shader/as blendable post processing effect. We could also render our scene to an offscreen render buffer/texture and map this texture on a warped mesh.
UCLASS()
class AScreenRuntimeWarpMeshActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AScreenRuntimeWarpMeshActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadOnly)
	UProceduralMeshComponent *WarpedScreenMeshComponent;

	void GenerateWarpingMesh(unsigned int cols, unsigned int rows, bool warp, float hScale, float vScale);

private:
	TArray<FVector> Vertices;
	TArray<FVector2D> UVs;
	TArray<int32> Triangles;
};
