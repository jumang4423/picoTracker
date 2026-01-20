#ifndef _PC_TIMER_H_
#define _PC_TIMER_H_

#include "System/Timer/Timer.h"
#include <SDL.h>

class PCTimer : public I_Timer {
public:
    PCTimer();
    virtual ~PCTimer();
    
    virtual void SetPeriod(float msec);
    virtual bool Start();
    virtual void Stop();
    virtual float GetPeriod();
    
    // SDL Callback
    uint32_t OnTimerTick();

private:
    float period_;
    SDL_TimerID timerID_;
    bool running_;
};

class PCTimerService : public TimerService {
public:
    virtual I_Timer *CreateTimer();
    virtual void TriggerCallback(int msec, timerCallback cb);
};

#endif
