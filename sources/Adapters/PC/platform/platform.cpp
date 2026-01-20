
#include "platform.h"
#include <SDL.h>
#include <iostream>

void platform_init() {
    std::cout << "Platform Init (PC)" << std::endl;
}

void platform_reboot() {
    std::cout << "Platform Reboot (PC) - Exiting" << std::endl;
    exit(0);
}

void platform_bootloader() {
    std::cout << "Platform Bootloader (PC) - Exiting" << std::endl;
    exit(0);
}


class PCSysMutex : public SysMutex {
public:
    PCSysMutex() { mutex = SDL_CreateMutex(); }
    ~PCSysMutex() { SDL_DestroyMutex(mutex); }
    bool Lock() override { return SDL_LockMutex(mutex) == 0; }
    void Unlock() override { SDL_UnlockMutex(mutex); }
private:
    SDL_mutex *mutex;
};

SysMutex *platform_mutex() {
    return new PCSysMutex();
}


uint32_t millis(void) {
    return SDL_GetTicks();
}

uint32_t micros(void) {
    return SDL_GetPerformanceCounter() * 1000000 / SDL_GetPerformanceFrequency();
}

void platform_brightness(uint8_t value) {
    // No-op for PC
}

int16_t battery_health() {
    return 100; // Always full battery on PC
}
