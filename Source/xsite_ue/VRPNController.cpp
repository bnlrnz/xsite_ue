#include "VRPNController.h"

#include <sstream>

#include "Engine.h"
#include "xsite_ue.h"
#include "vrpn_WiiMote.h"

void VRPN_CALLBACK handle_dtrack_tracker(void *userData, const vrpn_TRACKERCB data)
{
    if (userData == nullptr)
        return;
    UVRPNController::TrackerData trackerData;
    trackerData.Pos[0] = data.pos[0];
    trackerData.Pos[1] = data.pos[1];
    trackerData.Pos[2] = data.pos[2];
    trackerData.Quat[0] = data.quat[0];
    trackerData.Quat[1] = data.quat[1];
    trackerData.Quat[2] = data.quat[2];
    trackerData.Quat[3] = data.quat[3];

    auto *thisptr = (UVRPNController *)userData;

    // store recent data
    thisptr->TrackerDataMap.Add(data.sensor, trackerData);

    // trigger all added callbacks
    thisptr->m_mutex.Lock();
    for (const auto &f : thisptr->GetTrackerChangedCallbacks())
    {
        //UE_LOG(LogCave, Warning, TEXT("Triggering tracker callback..."));
        //f.CheckCallable();
        f(data.sensor, trackerData);
    }
    thisptr->m_mutex.Unlock();
}

void VRPN_CALLBACK handle_dtrack_analog(void *userData, const vrpn_ANALOGCB data)
{
    if (userData == nullptr)
        return;

    auto *thisptr = (UVRPNController *)userData;

    // store recent data
    thisptr->AnalogDataField.NumChannels = data.num_channel;

    for (int i = 0; i < thisptr->AnalogDataField.NumChannels; ++i)
    {
        thisptr->AnalogDataField.Channel[i] = data.channel[i];
    }

    // trigger all added callbacks
    thisptr->m_mutex.Lock();
    for (const auto &f : thisptr->GetAnalogChangedCallbacks())
    {
        f(thisptr->AnalogDataField);
    }
    thisptr->m_mutex.Unlock();
}

void VRPN_CALLBACK handle_dtrack_button(void *userData, const vrpn_BUTTONCB data)
{
    if (userData == nullptr)
        return;

    auto *thisptr = (UVRPNController *)userData;

    // store recent data
    thisptr->ButtonDataMap.Add(data.button, data.state == 1 ? ButtonState::Pressed : ButtonState::Released);

    // trigger all added callbacks
    thisptr->m_mutex.Lock();
    for (const auto &f : thisptr->GetButtonPressedCallbacks())
    {
        f(data.button, data.state == 1 ? ButtonState::Pressed : ButtonState::Released);
    }
    thisptr->m_mutex.Unlock();
}

UVRPNController* UVRPNController::Create(const FString &Device, const FString &HostIP, uint32 Port){
    UVRPNController* Controller = NewObject<UVRPNController>();
    Controller->Init(Device, HostIP, Port);
    return Controller;
}

void UVRPNController::Init(const FString &Device, const FString &HostIP, uint32 Port)
{
    //UE_LOG(LogCave, Warning, TEXT("VRPNController connecting..."));

    std::stringstream nic;
    nic << TCHAR_TO_UTF8(*Device) << "@" << TCHAR_TO_UTF8(*HostIP) << ":" << Port;
    this->connection = vrpn_get_connection_by_name(nic.str().c_str());

    UE_LOG(LogCave, Warning, TEXT("VRPNController connecting %s... %s!"), *FString(nic.str().c_str()),
           this->connection->connected() ? TEXT("done") : TEXT("failed"));

    // create the tracker (marker + flystick) component and register the handler
    this->tracker = new vrpn_Tracker_Remote(TCHAR_TO_UTF8(*Device), connection);
    this->tracker->register_change_handler(this, handle_dtrack_tracker);

    // create the flystick analog stick
    this->analog = new vrpn_Analog_Remote(TCHAR_TO_UTF8(*Device), connection);
    this->analog->register_change_handler(this, handle_dtrack_analog);

    // create the flystick buttons
    this->button = new vrpn_Button_Remote(TCHAR_TO_UTF8(*Device), connection);
    this->button->register_change_handler(this, handle_dtrack_button);
}

void UVRPNController::Poll()
{
    if (tracker)
        tracker->mainloop();

    if (analog)
        analog->mainloop();

    if (button)
        button->mainloop();

    if (connection)
        connection->mainloop();

    // this blocks rendering :(
    //vrpn_SleepMsecs(20);
}

void UVRPNController::AddTrackerChangedCallback(const TFunction<void(int32, TrackerData)> &Callback)
{
    m_mutex.Lock();
    this->OnTrackerChangedCallbacks.Add(Callback);
    m_mutex.Unlock();
}

void UVRPNController::AddButtonPressedCallback(const TFunction<void(int32, ButtonState)> &Callback)
{
    m_mutex.Lock();
    this->OnButtonPressedCallbacks.Add(Callback);
    m_mutex.Unlock();
}

void UVRPNController::AddAnalogChangedCallback(const TFunction<void(AnalogData)> &Callback)
{
    m_mutex.Lock();
    this->OnAnalogChangedCallbacks.Add(Callback);
    m_mutex.Unlock();
}

const TArray<TFunction<void(int32, UVRPNController::TrackerData)>> &UVRPNController::GetTrackerChangedCallbacks()
{
    return this->OnTrackerChangedCallbacks;
}

const TArray<TFunction<void(int32, ButtonState)>> &UVRPNController::GetButtonPressedCallbacks()
{
    return this->OnButtonPressedCallbacks;
}

const TArray<TFunction<void(UVRPNController::AnalogData)>> &UVRPNController::GetAnalogChangedCallbacks()
{
    return this->OnAnalogChangedCallbacks;
}

UVRPNController::~UVRPNController()
{
    //UE_LOG(LogCave, Warning, TEXT("VRPNController destruction"));

    for (auto f : OnTrackerChangedCallbacks)
    {
        f = nullptr;
    }

    OnTrackerChangedCallbacks.Empty();

    if (this->tracker != nullptr)
    {
        delete (this->tracker);
    }
    if (this->button != nullptr)
    {
        delete (this->button);
    }
    if (this->analog != nullptr)
    {
        delete (this->analog);
    }
    if (this->connection != nullptr)
    {
        delete (this->connection);
    }
}
