#include "PCEventManager.h"
#include <iostream>
#include <SDL.h>

PCEventManager::PCEventManager() {}
PCEventManager::~PCEventManager() {}

int PCEventManager::MainLoop() {
    std::cout << "PCEventManager::MainLoop called (Stub)" << std::endl;
    return 0;
}

void PCEventManager::PostQuitMessage() {
    std::cout << "PCEventManager::PostQuitMessage called" << std::endl;
    SDL_Event event;
    event.type = SDL_QUIT;
    SDL_PushEvent(&event);
}

int PCEventManager::GetKeyCode(const char *name) {
    // Basic mapping for PC keys if needed
    // The framework asks for key code for config names like "KEY_A"
    return 0; 
}
