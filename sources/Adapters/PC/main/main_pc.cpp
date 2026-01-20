
#include <SDL.h>
#include <iostream>
#include "Adapters/PC/platform/platform.h"
#include "Adapters/PC/system/picoTrackerSystem.h"

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    platform_init();
    
    picoTrackerSystem::Boot(argc, argv);
    
    // Main loop will be inside picoTrackerSystem::MainLoop or handled here depending on design
    // For now, let's assume we call picoTrackerSystem::MainLoop()
    
    picoTrackerSystem::MainLoop();

    picoTrackerSystem::Shutdown();
    SDL_Quit();

    return 0;
}
