// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "xsite_ue.h"

#if PLATFORM_WINDOWS
// put this at the top of your .h file above #includes
// UE4: allow Windows platform types to avoid naming collisions
// must be undone at the bottom of this file!
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#endif

#include "include/vrpn_Connection.h" // for vrpn_Connection, etc
#include "include/vrpn_Shared.h"     // for vrpn_SleepMsecs
#include "include/vrpn_Tracker.h"
#include "include/vrpn_Button.h"
#include "include/vrpn_Analog.h"

#include "Containers/HashTable.h"

#define ANALOG_CHANNELS 2 // we only need 2 channels for our flystick

UENUM(BlueprintType)
enum ButtonState
{
    Pressed = 1,
    Released = 0
};

class VRPNController
{  
public:
    typedef struct _TrackerData
    {
        double Pos[3];
        double Quat[4];
    } TrackerData;

    typedef struct _AnalogData
    {
        int NumChannels;
        double Channel[ANALOG_CHANNELS];
    } AnalogData;

    // stores the recently received data
    TMap<int32, VRPNController::TrackerData> TrackerDataMap;
    TMap<int32, ButtonState> ButtonDataMap;
    AnalogData AnalogDataField{};

    void AddTrackerChangedCallback(const TFunction<void(int32, TrackerData)> &Callback);
    void AddButtonPressedCallback(const TFunction<void(int32, ButtonState)> &Callback);
    void AddAnalogChangedCallback(const TFunction<void(AnalogData)> &Callback);

    const TArray<TFunction<void(int32, TrackerData)>> &GetTrackerChangedCallbacks();
    const TArray<TFunction<void(int32, ButtonState)>> &GetButtonPressedCallbacks();
    const TArray<TFunction<void(AnalogData)>> &GetAnalogChangedCallbacks();

    FCriticalSection m_mutex;

private:
    // triggered when new data arrives
    TArray<TFunction<void(int32, TrackerData)>> OnTrackerChangedCallbacks;
    TArray<TFunction<void(int32, ButtonState)>> OnButtonPressedCallbacks;
    TArray<TFunction<void(AnalogData)>> OnAnalogChangedCallbacks;

    vrpn_Connection *connection = nullptr;

    vrpn_Tracker_Remote *tracker = nullptr;
    vrpn_Button_Remote *button = nullptr;
    vrpn_Analog_Remote *analog = nullptr;

public:
    VRPNController(const FString &Device, const FString &HostIP, uint32 Port = vrpn_DEFAULT_LISTEN_PORT_NO);

    // run this whenever a new tracker value is needed, e.g. in the Tick Method, then access the value by the TrackerDataMap
    void Poll();

    ~VRPNController();
};

#if PLATFORM_WINDOWS
// put this at the bottom of the .h file
// UE4: disallow windows platform types
// this was enabled at the top of the file
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif