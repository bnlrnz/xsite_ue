// Fill out your copyright notice in the Description page of Project Settings.

#include "ScreenRuntimeWarpMeshActor.h"

//TODO make this come from config and choosen by screen name
const float Hi_x[] = {
    -255.98951119839865,
    0.6579372696594822,
    -0.0003026089636194544,
    -1.101532469740507e-05,
    4.332685389819662e-05,
    -1.2566635765139849e-05,
    1.1935859492155807e-09,
    7.132832413303441e-09,
    -6.4263851333624675e-09,
    2.82833240777572e-10};

const float Hi_y[] = {
    56.48524844438251,
    -0.055877090137070734,
    0.7348518447071067,
    3.276800010218201e-05,
    2.3651197733566443e-05,
    -3.375751157028628e-05,
    -8.574053699696848e-09,
    2.8455977126482097e-09,
    -1.8306525467661552e-09,
    1.420309266353241e-08};

const float Fdata[9] = {
    3594.349487713419,
    76.60356376314883,
    806.2689819335943,
    -117.57997576942103,
    3137.244007244394,
    240.0938262939469,
    -0.011985076415129137,
    0.0009892808353110794,
    1.0};

// Sets default values
AScreenRuntimeWarpMeshActor::AScreenRuntimeWarpMeshActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    WarpedScreenMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("WarpedScreenMeshComponent"));
    WarpedScreenMeshComponent->Mobility = EComponentMobility::Static;
    RootComponent = WarpedScreenMeshComponent;
}

// Called when the game starts or when spawned
void AScreenRuntimeWarpMeshActor::BeginPlay()
{
    Super::BeginPlay();

    GenerateWarpingMesh(16, 9, true, 2.0f, 2.0f);
}

// Called every frame
void AScreenRuntimeWarpMeshActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AScreenRuntimeWarpMeshActor::GenerateWarpingMesh(unsigned int cols, unsigned int rows, bool warp, float hScale, float vScale)
{
    Vertices.Empty();
    UVs.Empty();
    Triangles.Empty();
    WarpedScreenMeshComponent->ClearAllMeshSections();

    if (cols < 2 || rows < 2)
    {
        UE_LOG(LogCave, Error, TEXT("[GenerateWarpingMesh] Invalid parameters for cols/rows"));
        return;
    }

    const unsigned int NumVertices = cols * rows;

    float sl[2], tl[2], s[2], t[2], cx = 0, cy = 0, tx = 0, ty = 0;
    s[0] = 0.0f;
    t[0] = 0.0f;
    s[1] = 1.0f;
    t[1] = 1.0f;

    // reference points screen
    sl[0] = 0.0f;
    tl[0] = 0.0f;
    sl[1] = 0.564113f;//1.0f;
    tl[1] = 0.370322f;//1.0f;

    cx = (sl[0] - sl[1]) / (s[0] - s[1]);
    cy = (tl[0] - tl[1]) / (t[0] - t[1]);
    tx = sl[0] - cx * s[0];
    ty = tl[0] - cy * t[0];

    float F[3][3] = {
        {Fdata[0] * cx, Fdata[1] * cy, Fdata[0] * tx + Fdata[1] * ty + Fdata[2]},
        {Fdata[3] * cx, Fdata[4] * cy, Fdata[3] * tx + Fdata[4] * ty + Fdata[5]},
        {Fdata[6] * cx, Fdata[7] * cy, Fdata[6] * tx + Fdata[7] * ty + Fdata[8]}};

    float xStep = 1.0f / (static_cast<float>(cols) - 1.0f);
    float yStep = 1.0f / (static_cast<float>(rows) - 1.0f);

    for (unsigned int y = 0, i = 0; y < rows; ++y)
    {
        for (unsigned int x = 0; x < cols; ++x, ++i)
        {
            FVector V(0, 0, 0);

            V.X = static_cast<float>(x) * xStep;
            V.Y = static_cast<float>(y) * yStep;
            V.Z = 1.0f;

            UVs.Add(FVector2D(static_cast<float>(x) * xStep, !warp ? 1.0f - static_cast<float>(y) * yStep : static_cast<float>(y) * yStep));

            if (warp)
            {
                //V = V * F;

                FVector VF(
                    F[0][0] * V.X + F[0][1] * V.Y + F[0][2] * V.Z,
                    F[1][0] * V.X + F[1][1] * V.Y + F[1][2] * V.Z,
                    F[2][0] * V.X + F[2][1] * V.Y + F[2][2] * V.Z);

                V = VF;
                V = V / V.Z;

                float X   = Hi_x[0] + Hi_x[1]*V.X + Hi_x[2]*V.Y + Hi_x[3]*V.X*V.Y + Hi_x[4]*V.X*V.X + Hi_x[5]*V.Y*V.Y
                        + Hi_x[6]*V.X*V.X*V.Y + Hi_x[7]*V.X*V.Y*V.Y + Hi_x[8]*V.X*V.X*V.X + Hi_x[9]*V.Y*V.Y*V.Y;
                float Y   = Hi_y[0] + Hi_y[1]*V.X + Hi_y[2]*V.Y + Hi_y[3]*V.X*V.Y + Hi_y[4]*V.X*V.X + Hi_y[5]*V.Y*V.Y
                        + Hi_y[6]*V.X*V.X*V.Y + Hi_y[7]*V.X*V.Y*V.Y + Hi_y[8]*V.X*V.X*V.X + Hi_y[9]*V.Y*V.Y*V.Y;

                V.X = X / 1920.0f;
                V.Y = Y / 1080.0f;
                
                //UE_LOG(LogCave, Error, TEXT("[GenerateWarpingMesh] %2d, %2d Vertices.Add %4.5f, %4.5f"), x, y, V.X, V.Y);

            }

            V.X = (V.X - 0.5f) * hScale * 100;
            V.Y = (V.Y - 0.5f) * -vScale * 100;

            Vertices.Add(V);
        }
    }

/*
    for (unsigned ri = 0; ri < rows - 1; ++ri)
    {
        for (unsigned ci = 0; ci < cols - 1; ++ci)
        {
            //  D----------/C
            //  |        /  |
            //  |      /    |
            //  |    /      |
            //  |  /        |
            //  A/----------B
            //

            unsigned int a = (cols) * (ci + 1) + ri;
            unsigned int b = (cols) * (ci + 1) + ri - cols;
            unsigned int c = b + 1;
            unsigned int d = a + 1;

            Triangles.Add(a);
            Triangles.Add(b);
            Triangles.Add(c);

            Triangles.Add(a);
            Triangles.Add(c);
            Triangles.Add(d);
        }
    }
*/

	for (uint32 y = 0; y < rows - 1; y++)
	{
		for (uint32 x = 0; x < cols - 1; x++)
		{
			Triangles.Add(x + (y * cols));					//current vertex
			Triangles.Add(x + (y * cols) + cols);			//current vertex + row
			Triangles.Add(x + (y * cols) + cols + 1);		//current vertex + row + one right

			Triangles.Add(x + (y * cols));					//current vertex
			Triangles.Add(x + (y * cols) + cols + 1);		//current vertex + row + one right
			Triangles.Add(x + (y * cols) + 1);				//current vertex + one right
		}
	}
    
    //UE_LOG(LogCave, Warning, TEXT("Creating Mesh with: %d Vertices, %d Triangles, %d UVs"), Vertices.Num(), Triangles.Num() / 3, UVs.Num());

    WarpedScreenMeshComponent->CreateMeshSection_LinearColor(0, Vertices, Triangles, TArray<FVector>(), UVs, TArray<FLinearColor>(),
                                                             TArray<FProcMeshTangent>(), false);

}
