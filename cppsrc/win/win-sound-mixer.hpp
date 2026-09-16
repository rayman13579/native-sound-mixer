#pragma once

#include <Audioclient.h>
#include <audiopolicy.h>
#include <endpointvolume.h>
#include <mmdeviceapi.h>
#include <string>
#include <vector>
#include "sound-mixer-utils.hpp"

#define LEFT 0
#define RIGHT 1

using SoundMixerUtils::DeviceDescriptor;
using SoundMixerUtils::DeviceType;
using SoundMixerUtils::AudioSessionDescriptor;
using SoundMixerUtils::NotificationHandler;
using SoundMixerUtils::VolumeBalance;

namespace WinSoundMixer
{

typedef void (*on_device_changed_cb_t)(
  DeviceDescriptor dev, NotificationHandler);
typedef void (*on_audio_session_changed_cb_t)(
  AudioSessionDescriptor session, NotificationHandler);

class AudioSessionEvents;

class AudioSession {
    friend class AudioSessionEvents;

  public:
    AudioSession(IAudioSessionControl2 *control);
    ~AudioSession();

    virtual bool GetMute();
    virtual void SetMute(bool);
    virtual float GetVolume();
    virtual void SetVolume(float);

    virtual void SetVolumeBalance(const VolumeBalance &);
    virtual VolumeBalance GetVolumeBalance();

    std::string id();
    std::string name();
    std::string path();
    AudioSessionState state();
    AudioSessionDescriptor Desc();
    bool RegisterEventCallback(on_audio_session_changed_cb_t cb);

  protected:
    IAudioSessionControl2 *control;
    on_audio_session_changed_cb_t _sessionCallback;
    AudioSessionEvents *sessionEvents;
    bool eventsRegistered;
    float _oldVolume;
    BOOL _oldMute;

    ISimpleAudioVolume *getAudioVolume();
};

class Device {
  public:
    Device(IMMDevice *, on_device_changed_cb_t cb);
    ~Device();

    virtual bool GetMute();
    virtual void SetMute(bool);
    virtual float GetVolume();
    virtual void SetVolume(float);

    virtual void SetVolumeBalance(const VolumeBalance &);
    virtual VolumeBalance GetVolumeBalance();

    /**
     *  \brief      Updates the info of the Device.
     */
    virtual bool Update();

    /**
     *  \brief      Returns whether the device is still available.
     */
    bool IsValid();

    DeviceDescriptor Desc()
    {
        return desc;
    }

    std::vector<AudioSession *> GetAudioSessions();
    AudioSession *GetAudioSessionById(std::string);

    static IMMDeviceEnumerator *GetEnumerator();
    on_device_changed_cb_t _deviceCallback;

    float _oldVolume = 0.F;
    BOOL _oldMute = 0.F;

  protected:
    IMMDevice *device;
    IMMEndpoint *endpoint;
    DeviceDescriptor desc;
    IAudioEndpointVolume *endpointVolume;
    IAudioEndpointVolumeCallback *device_cb;

  private:
    bool valid = true;
};

class SoundMixer {
  public:
    SoundMixer(on_device_changed_cb_t cb);
    ~SoundMixer();
    std::vector<Device *> GetDevices();
    Device *GetDefaultDevice(DeviceType);

  private:
    IMMDeviceEnumerator *pEnumerator = nullptr;
    std::map<std::string, Device *> devices;

  private:
    /**
     *  \brief  Returns the corresponding device if already in the list,
     *  NULL otherwise.
     *
     *  \fn     Device *getDeviceById(LPWSTR id);
     *  \param id   The id of the device to find.
     *  \returns    The corresponding device if already in the list, NULL
     *  otherwise.
     *  \remarks    The caller is responsible for freeing the provided id.
     */
    Device *getDeviceById(LPWSTR id);

    void filterDevices();
    on_device_changed_cb_t deviceCallback;
};

class SoundMixerAudioEndpointVolumeCallback
    : public IAudioEndpointVolumeCallback {
  public:
    SoundMixerAudioEndpointVolumeCallback(Device *dev);

    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

  private:
    IFACEMETHODIMP OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify);
    IFACEMETHODIMP QueryInterface(const IID &iid, void **ppUnk);

  private:
    Device *device;
};

class AudioSessionEvents : public IAudioSessionEvents {
  public:
    AudioSessionEvents(AudioSession *session);

    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

  private:
    IFACEMETHODIMP OnDisplayNameChanged(LPCWSTR, LPCGUID);
    IFACEMETHODIMP OnIconPathChanged(LPCWSTR, LPCGUID);
    IFACEMETHODIMP OnSimpleVolumeChanged(float, BOOL, LPCGUID);
    IFACEMETHODIMP OnChannelVolumeChanged(
        DWORD, float[], DWORD, LPCGUID);
    IFACEMETHODIMP OnGroupingParamChanged(LPCGUID, LPCGUID);
    IFACEMETHODIMP OnStateChanged(AudioSessionState);
    IFACEMETHODIMP OnSessionDisconnected(AudioSessionDisconnectReason);
    IFACEMETHODIMP QueryInterface(const IID &, void **);

  private:
    AudioSession *session;
};
}; // namespace WinSoundMixer
