
#include "ili9341.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

static uint16_t frameBuffer[320 * 240];
static uint16_t window_x0 = 0, window_x1 = 0, window_y0 = 0, window_y1 = 0;
static uint16_t cursor_x = 0, cursor_y = 0;
static uint8_t rotation = 0;

uint16_t *GetPCDisplayBuffer() {
    return frameBuffer;
}

void ili9341_init() {
    memset(frameBuffer, 0, sizeof(frameBuffer));
}

static uint8_t current_command = 0;

void ili9341_set_command(uint8_t command) {
    current_command = command;
    if (command == ILI9341_RAMWR) {
        cursor_x = window_x0;
        cursor_y = window_y0;
    }
}

void ili9341_command_param(uint8_t param) {
    if (current_command == ILI9341_MADCTL) {
        // Handle rotation if needed, but for now ignoring
    }
}

void ili9341_command_param16(uint16_t param) {
    if (current_command == ILI9341_CASET) {
        static bool first = true;
        if (first) { window_x0 = param; first = false; }
        else { window_x1 = param; first = true; }
    } else if (current_command == ILI9341_PASET) {
        static bool first = true;
        if (first) { window_y0 = param; first = false; }
        else { window_y1 = param; first = true; }
    }
}

void ili9341_start_writing() {
    // Preparing to write
}

void ili9341_stop_writing() {
    // Done writing
}

void ili9341_write_data_continuous(void *data, size_t len) {
    uint16_t *pixels = (uint16_t*)data;
    size_t count = len / 2;
    
    // Simple write logic respecting window
    // This assumes data is sent row by row or column by column depending on orientation
    // For picoTracker, orientation is 90 deg clockwise (row major?)
    
    // chargfx uses:
    // display_x = ILI9341_TFTHEIGHT - x - width; (rotated)
    // It mocks the hardware rotation.
    
    // Note: chargfx sets window then writes bytes.
    // We just write into the buffer at cursor_x, cursor_y
    
    for (size_t i = 0; i < count; i++) {
        if (cursor_x < 320 && cursor_y < 240) {
            frameBuffer[cursor_y * 320 + cursor_x] = pixels[i];
        }
        
        cursor_x++;
        if (cursor_x > window_x1) {
            cursor_x = window_x0;
            cursor_y++;
        }
    }
}
