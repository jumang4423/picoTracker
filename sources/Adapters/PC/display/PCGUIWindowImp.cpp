
#include "PCGUIWindowImp.h"
#include "Application/Model/Config.h"
#include "UIFramework/BasicDatas/GUIPoint.h"
#include "UIFramework/SimpleBaseClasses/GUIWindow.h"
#include <cstdio>
#include <iostream>

// Mocking required Pico functions/types if not available
// Assuming chargfx.c handles drawing to the mock buffer.

extern "C" {
    uint16_t *GetPCDisplayBuffer(); 
}

// Global SDL resources
SDL_Window *g_window = nullptr;
SDL_Renderer *g_renderer = nullptr;
SDL_Texture *g_texture = nullptr;

// Helper to convert GUIColor to RGB565
static uint16_t to_rgb565(GUIColor &c) {
    return ((c._r & 0xF8) << 8) | ((c._g & 0xFC) << 3) | (c._b >> 3);
}

PCGUIWindowImp *PCGUIWindowImp::instance_ = nullptr;

PCGUIWindowImp::PCGUIWindowImp(GUICreateWindowParams &p) {
    instance_ = this;
    chargfx_init(); // Initialize mock display driver

    // Create SDL Window
    g_window = SDL_CreateWindow(p.title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                                320 * 2, 240 * 2, SDL_WINDOW_SHOWN); // 2x scale
    if (g_window) {
        g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
        if (g_renderer) {
            SDL_RenderSetLogicalSize(g_renderer, 320, 240);
            g_texture = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_RGB565, 
                                          SDL_TEXTUREACCESS_STREAMING, 320, 240);
        }
    }

    Config *config = Config::GetInstance();
    // Assuming we don't need RemoteUI for PC port initially
}

PCGUIWindowImp::~PCGUIWindowImp() {}

void PCGUIWindowImp::Clear(GUIColor &c, bool overlay) {
    chargfx_color_t backgroundColor = GetColor(c);
    chargfx_set_background(backgroundColor);
    chargfx_clear(backgroundColor);
}

void PCGUIWindowImp::SetColor(GUIColor &c) {
    chargfx_color_t color = GetColor(c);
    chargfx_set_foreground(color);
}

void PCGUIWindowImp::ClearTextRect(GUIRect &r) {
    // No-op or implementation if needed
}

void PCGUIWindowImp::DrawChar(const char c, GUIPoint &pos, GUITextProperties &p) {
    uint8_t x = pos._x / 8; // Assuming 8x8 font?
    uint8_t y = pos._y / 8;
    chargfx_set_cursor(x, y);
    chargfx_putc(c, p.invert_);
}

// Re-implementing GetColor and tracking lastIndex
static uint8_t lastColorIndex = 0;
static uint16_t lastPaletteRGB[16] = {0};

chargfx_color_t PCGUIWindowImp::GetColor(GUIColor &c) {
    if (c._paletteIndex >= 16) return CHARGFX_NORMAL;
    uint16_t rgb565 = to_rgb565(c);
    if (lastPaletteRGB[c._paletteIndex] != rgb565) {
        chargfx_set_palette_color(c._paletteIndex, rgb565);
        lastPaletteRGB[c._paletteIndex] = rgb565;
        // std::cout << "SetPalette " << (int)c._paletteIndex << " to " << std::hex << rgb565 << std::dec << std::endl;
    }
    lastColorIndex = c._paletteIndex; // Track it
    return (chargfx_color_t)c._paletteIndex;
}

void PCGUIWindowImp::DrawRect(GUIRect &r) {
     chargfx_fill_rect(lastColorIndex, r.Left(), r.Top(), r.Width(), r.Height());
}

GUIRect PCGUIWindowImp::GetRect() {
    return GUIRect(0, 0, 320, 240);
}

void PCGUIWindowImp::Invalidate() {
    // Request flush?
    // Use SDL_UserEvent to trigger main loop update or just flag it.
}

void PCGUIWindowImp::Lock() {}
void PCGUIWindowImp::Unlock() {}

void PCGUIWindowImp::Flush() {
    chargfx_draw_changed(); // Update the mock framebuffer

    // Now copy mock framebuffer to SDL texture
    uint16_t *buffer = GetPCDisplayBuffer();
    
    if (g_texture) {
        // Update texture
        // Lock texture? or UpdateTexture?
        // SDL_UpdateTexture(g_texture, NULL, buffer, 320 * sizeof(uint16_t));
        // Use lock for streaming
        void *pixels;
        int pitch;
        if (SDL_LockTexture(g_texture, NULL, &pixels, &pitch) == 0) {
            // Copy rows?
            // Assuming direct mapping 320x240
            // buffer is 320*240 uint16_t
            // pitch should be 320*2
            memcpy(pixels, buffer, 320 * 240 * 2);
            SDL_UnlockTexture(g_texture);
        } else {
             std::cerr << "SDL Lock Texture failed: " << SDL_GetError() << std::endl;
        }
        
        // Render
        SDL_RenderClear(g_renderer);
        SDL_RenderCopy(g_renderer, g_texture, NULL, NULL);
        SDL_RenderPresent(g_renderer);
    } else {
        std::cerr << "g_texture is null in Flush" << std::endl;
    }
}

void PCGUIWindowImp::PushEvent(GUIEvent &e) {
    if (_window) {
        _window->DispatchEvent(e);
    }
}

void PCGUIWindowImp::Update(Observable &o, I_ObservableData *d) {
    // Handle config changes
}
