#include "PCAudio.h"
#include <iostream>
#include "Services/Audio/AudioOutDriver.h"
#include "Services/Audio/AudioDriver.h"
#include "Services/Audio/AudioSettings.h"
#include <SDL.h>

class SDLAudioDriver : public AudioDriver {
public:
    SDLAudioDriver(AudioSettings &settings) : AudioDriver(settings) {}
    virtual ~SDLAudioDriver() {}

    bool InitDriver() override { return true; }
    void CloseDriver() override {}
    bool StartDriver() override { return true; }
    void StopDriver() override {}
    bool Interlaced() override { return true; }
    int GetPlayedBufferPercentage() override { return 0; }
    double GetStreamTime() override { return 0; }
};

PCAudio::PCAudio(AudioSettings &settings) : Audio(settings) {}

PCAudio::~PCAudio() {}

void PCAudio::Init() {
    std::cout << "PCAudio::Init called" << std::endl;
    // Create Driver
    // Use heap allocation to avoid destruction issues or static init issues
    // Note: This leaks memory but for this singleton it's acceptable for now
    SDLAudioDriver *driver = new SDLAudioDriver(settings_);
    AudioOutDriver *out = new AudioOutDriver(*driver);
    AddOutput(*out);
}


void PCAudio::Close() {
    std::cout << "PCAudio::Close called" << std::endl;
}

int PCAudio::GetSampleRate() {
    return 44100;
}
