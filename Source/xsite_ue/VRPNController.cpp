#include "VRPNController.h"

#include <sstream>

#include "Engine.h"
#include "xsite_ue.h"
#include "vrpn_WiiMote.h"

void VRPN_CALLBACK handle_dtrack_tracker(void *userData, const vrpn_TRACKERCB data)
{
    if (userData == nullptr)
        return;
    VRPNController::TrackerData trackerData;
    trackerData.Pos[0] = data.pos[0];
    trackerData.Pos[1] = data.pos[1];
    trackerData.Pos[2] = data.pos[2];
    trackerData.Quat[0] = data.quat[0];
    trackerData.Quat[1] = data.quat[1];
    trackerData.Quat[2] = data.quat[2];
    trackerData.Quat[3] = data.quat[3];

    auto *thisptr = (VRPNController *)userData;

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

    auto *thisptr = (VRPNController *)userData;

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

    auto *thisptr = (VRPNController *)userData;

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

void VRPNController::Init(const FString &Device, const FString &HostIP, uint32 Port)
{
    //UE_LOG(LogCave, Warning, TEXT("VRPNController connecting..."));

    std::stringstream nic;
    nic << TCHAR_TO_UTF8(*Device) << "@" << TCHAR_TO_UTF8(*HostIP) << ":" << Port;
    this->connection = std::shared_ptr<vrpn_Connection>(vrpn_get_connection_by_name(nic.str().c_str()));

    UE_LOG(LogCave, Warning, TEXT("VRPNController connecting %s... %s!"), *FString(nic.str().c_str()),
           this->connection->connected() ? TEXT("done") : TEXT("failed"));

    // create the tracker (marker + flystick) component and register the handler
    this->tracker = std::make_shared<vrpn_Tracker_Remote>(TCHAR_TO_UTF8(*Device), connection.get());
    this->tracker->register_change_handler(this, handle_dtrack_tracker);

    // create the flystick analog stick
    this->analog = std::make_shared<vrpn_Analog_Remote>(TCHAR_TO_UTF8(*Device), connection.get());
    this->analog->register_change_handler(this, handle_dtrack_analog);

    // create the flystick buttons
    this->button = std::make_shared<vrpn_Button_Remote>(TCHAR_TO_UTF8(*Device), connection.get());
    this->button->register_change_handler(this, handle_dtrack_button);
}

void VRPNController::Poll()
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

void VRPNController::AddTrackerChangedCallback(const TFunction<void(int32, TrackerData)> &Callback)
{
    m_mutex.Lock();
    this->OnTrackerChangedCallbacks.Add(Callback);
    m_mutex.Unlock();
}

void VRPNController::AddButtonPressedCallback(const TFunction<void(int32, ButtonState)> &Callback)
{
    m_mutex.Lock();
    this->OnButtonPressedCallbacks.Add(Callback);
    m_mutex.Unlock();
}

void VRPNController::AddAnalogChangedCallback(const TFunction<void(AnalogData)> &Callback)
{
    m_mutex.Lock();
    this->OnAnalogChangedCallbacks.Add(Callback);
    m_mutex.Unlock();
}

const TArray<TFunction<void(int32, VRPNController::TrackerData)>> &VRPNController::GetTrackerChangedCallbacks()
{
    return this->OnTrackerChangedCallbacks;
}

const TArray<TFunction<void(int32, ButtonState)>> &VRPNController::GetButtonPressedCallbacks()
{
    return this->OnButtonPressedCallbacks;
}

const TArray<TFunction<void(VRPNController::AnalogData)>> &VRPNController::GetAnalogChangedCallbacks()
{
    return this->OnAnalogChangedCallbacks;
}

VRPNController::~VRPNController()
{
    //UE_LOG(LogCave, Warning, TEXT("VRPNController destruction"));

    for (auto f : OnTrackerChangedCallbacks)
        f = nullptr;

    OnTrackerChangedCallbacks.Empty();

    for (auto f : OnAnalogChangedCallbacks)
        f = nullptr;

    OnAnalogChangedCallbacks.Empty();

    for (auto f : OnButtonPressedCallbacks)
        f = nullptr;
    
    OnButtonPressedCallbacks.Empty();

    this->tracker.reset();
    this->analog.reset();
    this->button.reset();
}
