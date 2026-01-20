
#include "picoTrackerSystem.h"
#include <SDL.h>
#include <iostream>
#include "Application/Application.h"
#include "System/Process/SysMutex.h"
#include "UIFramework/BasicDatas/GUIEvent.h"
#include "UIFramework/SimpleBaseClasses/GUIWindow.h"

bool picoTrackerSystem::invert_ = false;
unsigned int picoTrackerSystem::lastBeatCount_ = 0;
EventManager *picoTrackerSystem::eventManager_ = nullptr;

#include "../filesystem/PCFileSystem.h"
#include "../display/PCGUIFactory.h"
#include "../audio/PCAudio.h"
#include "Services/Midi/MidiService.h"

#include "../timer/PCTimer.h"
#include "../instruments/PCSamplePool.h"

void picoTrackerSystem::Boot(int argc, char **argv) {
    std::cout << "picoTrackerSystem Boot" << std::endl;
    
    // Install System
    System::Install(new picoTrackerSystem());

    // Install GUI Factory
    I_GUIWindowFactory::Install(new PCGUIFactory());
    
    // Install Timers
    TimerService::Install(new PCTimerService());

    // Install FileSystem
    static PCFileSystem pcFileSystem;
    FileSystem::Install(&pcFileSystem);

    // Install Audio
    AudioSettings hint;
    hint.bufferSize_ = 1024;
    hint.preBufferCount_ = 8;
    Audio::Install(new PCAudio(hint));
    
    // Install SamplePool
    SamplePool::Install(new PCSamplePool());

    // Install MidiService
    MidiService::Install(new MidiService());

    GUICreateWindowParams params;
    params.title = "picoTracker PC";
    Application::GetInstance()->Init(params);
}

void picoTrackerSystem::Shutdown() {
    std::cout << "picoTrackerSystem Shutdown" << std::endl;
}

int picoTrackerSystem::MainLoop() {
    std::cout << "picoTrackerSystem MainLoop Start" << std::endl;
    bool quit = false;
    SDL_Event e;
    
    Application *app = Application::GetInstance();
    if (!app) {
        std::cout << "Application instance is NULL!" << std::endl;
        return 1;
    }
    
    // Check window once
    if (!app->GetWindow()) {
         std::cout << "Application Window is NULL!" << std::endl;
    }

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
                GUIEventType type = (e.type == SDL_KEYDOWN) ? ET_PADBUTTONDOWN : ET_PADBUTTONUP;
                int button = EPBT_INVALID;
                
                switch(e.key.keysym.sym) {
                    case SDLK_LEFT: button = EPBT_LEFT; break;
                    case SDLK_RIGHT: button = EPBT_RIGHT; break;
                    case SDLK_UP: button = EPBT_UP; break;
                    case SDLK_DOWN: button = EPBT_DOWN; break;
                    case SDLK_z: button = EPBT_A; break;
                    case SDLK_x: button = EPBT_B; break;
                    case SDLK_RETURN: button = EPBT_START; break;
                    case SDLK_LSHIFT: button = EPBT_SELECT; break;
                    case SDLK_a: button = EPBT_L; break;
                    case SDLK_s: button = EPBT_R; break;
                }
                
                if (button != EPBT_INVALID) {
                     GUIEvent event(button, type, SDL_GetTicks(), false, false, false);
                     Application::GetInstance()->GetWindow()->PushEvent(event);
                }
                
                // Also handle special keys like Escape for quit?
                if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                     // Maybe bring up menu or quit?
                }
            }
        }
        
        // Trigger Redraw/Update if needed? 
        // Application::GetInstance()->GetWindow()->Invalidate(); // Only invalidates if needed
        // But for PC, Flush() draws to screen. 
        // We should call Flush() periodically or when invalidated.
        // SDL-based Flush() is in PCGUIWindowImp::Flush().
        // Does the framework call Flush()?
        // Usually EventManager loop calls Flush() on dirty.
        // Here we don't have EventManager loop.
        
        // Update Application/Window logic
        if (Application::GetInstance() && Application::GetInstance()->GetWindow()) {
            Application::GetInstance()->GetWindow()->AnimationUpdate();
            Application::GetInstance()->GetWindow()->Update(false); // Update logic, redraw if dirty
             
             // Then Flush to screen
            Application::GetInstance()->GetWindow()->Flush();
        }
        
        SDL_Delay(10); // Prevent 100% CPU
    }
    return 0;
}

// System overrides
unsigned long picoTrackerSystem::GetClock() { return SDL_GetTicks(); }
void picoTrackerSystem::GetBatteryState(BatteryState &state) {
    state.percentage = 100;
    state.voltage_mv = 4200;
    state.charging = true;
}
void picoTrackerSystem::SetDisplayBrightness(unsigned char value) {}
void picoTrackerSystem::PostQuitMessage() { 
    SDL_Event sdlevent;
    sdlevent.type = SDL_QUIT;
    SDL_PushEvent(&sdlevent);
}
unsigned int picoTrackerSystem::GetMemoryUsage() { return 0; }
void picoTrackerSystem::SystemBootloader() { exit(0); }
void picoTrackerSystem::SystemReboot() { exit(0); }
void picoTrackerSystem::SystemPutChar(int c) { putchar(c); }
uint32_t picoTrackerSystem::GetRandomNumber() { return rand(); }
uint32_t picoTrackerSystem::Micros() { return SDL_GetPerformanceCounter() * 1000000 / SDL_GetPerformanceFrequency(); }
uint32_t picoTrackerSystem::Millis() { return SDL_GetTicks(); }

void picoTrackerSystem::Sleep(int millisec) {
    SDL_Delay(millisec);
}

