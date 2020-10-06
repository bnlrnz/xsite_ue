// Fill out your copyright notice in the Description page of Project Settings.

#include "VRPNFlystickActor.h"
#include "DrawDebugHelpers.h"
#include "CaveGameModeBase.h"
#include "Engine/StaticMeshSocket.h"
#include "FlystickManipulatable.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Engine.h"

/*

 [Flystick]
Type=Tracker
Tracker = (Id=18 Name=Flystick Description="CAVE Flystick Tracker" PlayerId=0 Hand=Right)
Address = DTrack@127.0.0.1:3883
FlipZAxis = False
RotationOffset = (X=1.0 Y=1.0 Z=0.0 Angle=0)
PositionOffset = (X=0.0 Y=-1.25 Z=-1.5)
TrackerUnitsToUE4Units = 100

 vrpn / cave coordinates -> x and y are flipped!

*/

// Sets default values
AVRPNFlystickActor::AVRPNFlystickActor()
{
    SetReplicates(true);
    bAlwaysRelevant = true;

    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = static_cast<uint8>(true);

    this->WallPositions.Add(this->LeftWallPosition);
    this->WallPositions.Add(this->RightWallPosition);
    this->WallPositions.Add(this->FrontWallPosition);
    this->WallPositions.Add(this->FloorPosition);

    this->WallNormals.Add(this->LeftWallNormal);
    this->WallNormals.Add(this->RightWallNormal);
    this->WallNormals.Add(this->FrontWallNormal);
    this->WallNormals.Add(this->FloorNormal);
}

//DEBUG:
static TAutoConsoleVariable<float> PitchOffset(
    TEXT("cave.FlystickActor.PitchOffset"),
    0.0f,
    TEXT(""),
    ECVF_Scalability | ECVF_RenderThreadSafe);
static TAutoConsoleVariable<float> YawOffset(
    TEXT("cave.FlystickActor.YawOffset"),
    0.0f,
    TEXT(""),
    ECVF_Scalability | ECVF_RenderThreadSafe);
static TAutoConsoleVariable<float> RollOffset(
    TEXT("cave.FlystickActor.RollOffset"),
    0.0f,
    TEXT(""),
    ECVF_Scalability | ECVF_RenderThreadSafe);

static TAutoConsoleVariable<float> MoveSpeed(
    TEXT("cave.FlystickActor.MoveSpeed"),
    10.0f,
    TEXT(""),
    ECVF_Scalability | ECVF_RenderThreadSafe);

// Called when the game starts or when spawned
void AVRPNFlystickActor::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogCave, Warning, TEXT("Spawning Flystick on %s"), HasAuthority() ? TEXT("Server") : TEXT("Client"));

    // only on server!
    if (!GetWorld()->IsServer())
        return;

    auto CaveGameInstance = Cast<UCaveGameInstance>(GetWorld()->GetGameInstance());
    auto vrpnController = CaveGameInstance->GetCaveController()->GetVRPNController();

    FVector EyeOrigin = FVector(0,0,0);

    if (CaveGameInstance->GetCaveController() != nullptr)
    {
        EyeOrigin = CaveGameInstance->GetCaveController()->EyeOrigin;
    }

    if (vrpnController == nullptr)
    {
        UE_LOG(LogCave, Warning, TEXT("[AVRPNFlystickActor::BeginPlay] Could not obtain VrpnController from CaveController or GameInstance"));
        return;
    }

    vrpnController->AddTrackerChangedCallback(
        [this, vrpnController, EyeOrigin](int32 sensor, VRPNController::TrackerData trackerData) {
            //UE_LOG(LogCave, Warning, TEXT("Tracker ID: %d"), sensor);
            // 18 = Flystick, 15 = WiiTarget aka Flystick substitute
            if (sensor != 18 && sensor != 15)
                return;

            float xOffset = -EyeOrigin.X;
            float yOffset = -EyeOrigin.Y;
            float zOffset = -EyeOrigin.Z;

            this->Start = FVector(
                (float)(trackerData.Pos[1] + xOffset) * 100.f,
                (float)(trackerData.Pos[0] + yOffset) * 100.f,
                (float)(trackerData.Pos[2] + zOffset) * 100.f);

            // this is exactly our case
            //https://stackoverflow.com/questions/39235842/switch-axes-and-handedness-of-a-quaternion
            FRotator FlystickOrientation(
                FQuat(
                    -1.0f * (float)trackerData.Quat[1],
                    -1.0f * (float)trackerData.Quat[0],
                    -1.0f * (float)trackerData.Quat[2],
                    (float)trackerData.Quat[3]));

            FlystickOrientation = FRotator(
                FlystickOrientation.Pitch + PitchOffset.GetValueOnGameThread(),
                FlystickOrientation.Yaw + YawOffset.GetValueOnGameThread(),
                FlystickOrientation.Roll + RollOffset.GetValueOnGameThread());

            //UE_LOG(LogCave, Warning, TEXT("%d - %s"), sensor, *FlystickOrientation.ToString());

            this->End = FlystickOrientation.RotateVector(FVector(500, 0, 0)) + this->Start;

            this->Start = this->CaveHeadCharacter->NetRot.RotateVector(this->Start);
            this->End = this->CaveHeadCharacter->NetRot.RotateVector(this->End);
        });

    vrpnController->AddButtonPressedCallback(
        [this, vrpnController](int32 button, ButtonState state) {
            /* WIImote
                 * 0 - home
                 * 1 - 1
                 * 2 - 2
                 * 3 - A
                 * 4 - Trigger
                 * 5 - -
                 * 6 - +
                 * 7 - left
                 * 8 - right
                 * 9 - down
                 * 10- up
                 *
                 * Flystick
                 * 0 - Trigger
                 * 1 - 4/4 Button row
                 * 2 - 3/4 Button row
                 * 3 - 2/4 Button row
                 * 4 - 1/4 Button row
                 * 6 - analog stick press
                 */

            if (button == 0)
            {
                bPrimaryClick = true;
                PrimaryButtonState = state;
            }

            if (button == 4)
                FlystickMode = Manipulate;

            if (button == 3)
                FlystickMode = Navigate;
        });

    vrpnController->AddAnalogChangedCallback(
        [this, vrpnController](VRPNController::AnalogData analogData) {
            //TODO: we got exactly two channels, nonetheless this should be more generic
            this->AnalogX = analogData.Channel[0]; // left right
            this->AnalogY = analogData.Channel[1]; // up down
        });
}

// Called every frame
void AVRPNFlystickActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // get the cave head - we need it basically everywhere
    if (CaveHeadCharacter == nullptr)
    {
        auto *CaveGameInstance = (UCaveGameInstance *)GetWorld()->GetGameInstance();
        this->CaveHeadCharacter = CaveGameInstance->GetCaveHeadCharacter();
    }

    switch (FlystickMode)
    {
    case Manipulate:
        HandleManipulate();
        break;
    case Navigate:
        HandleNavigate();
        break;
    }

    Multicast_UpdatePointer(Start, End, bIsHit);

    FVector IntersectionPoint = this->Start + this->CaveHeadCharacter->NetLoc;

    if (bShowLaser)
    {
        for (int i = 0; i < this->WallPositions.Num(); ++i)
        {
            FVector RotatedWallLocation = this->WallPositions[i];
            FVector RotatedWallNormal = this->WallNormals[i];

            if (IsValid(this->CaveHeadCharacter->GetController()))
            {
                RotatedWallLocation = this->CaveHeadCharacter->NetRot.RotateVector(
                    this->WallPositions[i]);
                RotatedWallNormal = this->CaveHeadCharacter->NetRot.RotateVector(
                    this->WallNormals[i]);
            }

            if (FMath::SegmentPlaneIntersection(
                    this->Start + this->CaveHeadCharacter->NetLoc,
                    this->End + this->CaveHeadCharacter->NetLoc,
                    FPlane(this->CaveHeadCharacter->NetLoc, RotatedWallLocation, RotatedWallNormal),
                    IntersectionPoint))
            {
                break;
            }
        }

        //DrawDebugSphere(GetWorld(),  IntersectionPoint, 5.0f, 8, FColor::Red, false, 0.2f);
        DrawDebugLine(
            GetWorld(),
            IntersectionPoint,
            this->End + this->CaveHeadCharacter->NetLoc,
            bIsHit ? FColor::Green : FColor::Red,
            false,
            bIsHit ? 0.06f : 0.03f, 0, 1.0f);
    }

    this->bPrimaryClick = false;
}

void AVRPNFlystickActor::Multicast_UpdatePointer_Implementation(FVector NetStart, FVector NetEnd, bool NetbIsHit)
{
    this->Start = NetStart;
    this->End = NetEnd;
    this->bIsHit = NetbIsHit;
}


void AVRPNFlystickActor::HandleManipulate()
{

    // only on server, everything is replicated to the clients
    if (!GetWorld()->IsServer())
        return;

    // laser collision handling
    FHitResult OutHit;
    FCollisionQueryParams CollisionParameters;

    bool Hit = GetWorld()->LineTraceSingleByChannel(
        OutHit,
        this->Start + this->CaveHeadCharacter->NetLoc,
        this->End + this->CaveHeadCharacter->NetLoc,
        ECC_Visibility,
        CollisionParameters);

    if (Hit)
    {
        if (OutHit.bBlockingHit)
        {
            // did we hit another actor?
            if (this->PointedActor != OutHit.GetActor())
            {
                //trigger deselect callback for the old actor
                DeselectPointedActor();
                this->PointedActor = OutHit.GetActor();
            }

            //UE_LOG(LogCave, Warning, TEXT("Hit %s - PrimaryClick: %s - PrimaryPressed: %s"),
            //       *OutHit.GetActor()->GetName(), bPrimaryClick ? TEXT("true") : TEXT("false"),
            //       PrimaryButtonState == ButtonState::Pressed ? TEXT("Pressed") : TEXT("Released"));

            // trigger the interface implementations
            if (OutHit.GetActor()->GetClass()->ImplementsInterface(UFlystickManipulatable::StaticClass()))
            {

                this->bIsHit = true;

                IFlystickManipulatable::Execute_SelectedCallback(OutHit.GetActor(), this, OutHit.ImpactPoint);

                if (bPrimaryClick)
                {
                    this->AttachedActorName = OutHit.GetActor()->GetName();
                    //UE_LOG(LogCave, Warning, TEXT("Flystick - Clicked Event: %s"), *this->AttachedActorName);
                    IFlystickManipulatable::Execute_ClickedAtCallback(OutHit.GetActor(), this, OutHit.ImpactPoint,
                                                                      PrimaryButtonState);
                    this->bPrimaryClick = false;
                }
            }
            else
            {
                // did not hit an actor with the flystick interface
                this->bIsHit = false;
            }
        }
        else
        {
            DeselectPointedActor();
            this->PointedActor = nullptr;
            this->bIsHit = false;
        }
    }
    else
    {
        DeselectPointedActor();
        this->PointedActor = nullptr;
        this->bIsHit = false;
    }

    if (AnalogX != 0)
    {
        this->LaserOffset += AnalogX;
    }

    if (AnalogY != 0)
    {
        this->LaserOffset += AnalogY;
    }

    double oldLen = (this->Start + this->End).Size();
    this->End = (this->LaserOffset + oldLen) / oldLen * this->End;
}

void AVRPNFlystickActor::HandleNavigate()
{
    // only on server, the cave head rotation is replicated to the clients
    if (!GetWorld()->IsServer())
        return;

    // forwards/backwards
    if (this->AnalogY != 0.0f)
    {
        FVector Direction = (this->End - this->Start);
        Direction = Direction / Direction.Size();
        this->CaveHeadCharacter->SetActorLocation(
            this->CaveHeadCharacter->NetLoc +
            Direction * this->AnalogY * MoveSpeed.GetValueOnGameThread());
    }

    // strafe
    if (this->AnalogX != 0.0f)
    {
        FVector Direction = (this->End - this->Start);
        Direction = FVector::CrossProduct(Direction, FVector::UpVector);
        Direction = Direction / Direction.Size();
        this->CaveHeadCharacter->SetActorLocation(
            this->CaveHeadCharacter->NetLoc +
            Direction * -this->AnalogX * MoveSpeed.GetValueOnGameThread());
    }

    // no drag, no rotate
    if (PrimaryButtonState != ButtonState::Pressed)
        return;

    FVector Direction = (this->End - this->Start);
    Direction = Direction / Direction.Size();

    // start rotation drag
    if (bPrimaryClick)
    {
        this->CaveHeadCharacterStartRotation = this->CaveHeadCharacter->NetRot;
        this->DragStartRotation = Direction.Rotation();
    }

    // rotate
    this->CaveHeadCharacter->GetController()->SetControlRotation(
        this->DragStartRotation - Direction.Rotation() + this->CaveHeadCharacterStartRotation);

    this->CaveHeadCharacter->SetActorRotation(
        this->DragStartRotation - Direction.Rotation() + this->CaveHeadCharacterStartRotation,
        ETeleportType::None);
}

void AVRPNFlystickActor::DeselectPointedActor()
{
    if (this->PointedActor == nullptr)
        return;
    if (this->PointedActor->GetClass()->ImplementsInterface(UFlystickManipulatable::StaticClass()))
    {
        IFlystickManipulatable::Execute_DeselectedCallback(this->PointedActor, this);
    }
}