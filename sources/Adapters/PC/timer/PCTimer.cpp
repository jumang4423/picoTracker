#include "PCTimer.h"
#include "System/Console/Trace.h"
#include <iostream>

// SDL user data for callback
struct TimerCallbackData {
    PCTimer *timer;
};

static uint32_t SDLTimerCallback(uint32_t interval, void *param) {
    PCTimer *timer = (PCTimer *)param;
    return timer->OnTimerTick();
}

static uint32_t SDLTriggerCallback(uint32_t interval, void *param) {
    timerCallback cb = (timerCallback)param;
    if (cb) {
        cb();
    }
    return 0; // 0 means stop the timer (run once)
}

PCTimer::PCTimer() : period_(-1.0f), timerID_(0), running_(false) {}

PCTimer::~PCTimer() {
    Stop();
}

void PCTimer::SetPeriod(float msec) {
    period_ = msec;
}

bool PCTimer::Start() {
    Stop();
    if (period_ > 0) {
        timerID_ = SDL_AddTimer((uint32_t)period_, SDLTimerCallback, this);
        running_ = (timerID_ != 0);
        if (!running_) {
            Trace::Error("SDL_AddTimer failed: %s", SDL_GetError());
        }
    }
    return running_;
}

void PCTimer::Stop() {
    if (timerID_ != 0) {
        SDL_RemoveTimer(timerID_);
        timerID_ = 0;
    }
    running_ = false;
}

float PCTimer::GetPeriod() {
    return period_;
}

uint32_t PCTimer::OnTimerTick() {
    if (running_) {
        SetChanged();
        NotifyObservers();
    }
    return (uint32_t)period_; // Return next interval
}

// Service Implementation

I_Timer *PCTimerService::CreateTimer() {
    static PCTimer timerInstance;
    timerInstance.Stop();
    timerInstance.SetPeriod(-1.0f);
    return &timerInstance;
}

void PCTimerService::TriggerCallback(int msec, timerCallback cb) {
    if (SDL_AddTimer(msec, SDLTriggerCallback, (void*)cb) == 0) {
        Trace::Error("SDL_AddTimer for TriggerCallback failed: %s", SDL_GetError());
    }
}
