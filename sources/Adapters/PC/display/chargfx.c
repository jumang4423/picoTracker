/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "chargfx.h"
#include "font.h"
#include <assert.h>
// #include "hardware/spi.h"
// #include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

/* Character graphics mode */

#define SWAP_BYTES(color) ((uint16_t)(color >> 8) | (uint16_t)(color << 8))

static chargfx_color_t screen_bg_color = CHARGFX_BG;
static chargfx_color_t screen_fg_color = CHARGFX_NORMAL;
static int cursor_x = 0;
static int cursor_y = 0;
static uint8_t screen[TEXT_HEIGHT * TEXT_WIDTH] = {0};
static uint8_t colors[TEXT_HEIGHT * TEXT_WIDTH] = {0};
static uint16_t buffer[CHAR_HEIGHT * CHAR_WIDTH * BUFFER_CHARS] = {0};

static uint8_t ui_font_index = 0;

// Using a bit array in order to save memory, there is a slight performance
// hit in doing so vs a bool array
static uint8_t changed[TEXT_HEIGHT * TEXT_WIDTH / 8] = {0};
#define SetBit(A, k) (A[(k) / 8] |= (1 << ((k) % 8)))
#define ClearBit(A, k) (A[(k) / 8] &= ~(1 << ((k) % 8)))
#define TestBit(A, k) (A[(k) / 8] & (1 << ((k) % 8)))

// Default palette, can be redefined
static uint16_t palette[16] = {
    SWAP_BYTES(0x0000), SWAP_BYTES(0x49E5), SWAP_BYTES(0xB926),
    SWAP_BYTES(0xE371), SWAP_BYTES(0x9CF3), SWAP_BYTES(0xA324),
    SWAP_BYTES(0xEC46), SWAP_BYTES(0xF70D), SWAP_BYTES(0xffff),
    SWAP_BYTES(0x1926), SWAP_BYTES(0x2A49), SWAP_BYTES(0x4443),
    SWAP_BYTES(0xA664), SWAP_BYTES(0x02B0), SWAP_BYTES(0x351E),
    SWAP_BYTES(0xB6FD)};

void chargfx_clear(chargfx_color_t color) {
  int size = TEXT_WIDTH * TEXT_HEIGHT;
  memset(screen, 0, size);
  memset(colors, color, size);
  chargfx_set_cursor(0, 0);
  chargfx_draw_screen();
}

void chargfx_set_foreground(chargfx_color_t color) { screen_fg_color = color; }

void chargfx_set_background(chargfx_color_t color) { screen_bg_color = color; }

void chargfx_set_font_index(uint8_t idx) { ui_font_index = idx; }

void chargfx_set_cursor(uint8_t x, uint8_t y) {
  cursor_x = x;
  cursor_y = y;
}

uint8_t chargfx_get_cursor_x() { return cursor_x; }

uint8_t chargfx_get_cursor_y() { return cursor_y; }

void chargfx_putc(char c, bool invert) {
  int idx = cursor_y * TEXT_WIDTH + cursor_x;
  if (c >= 32 && c <= 127) {
    screen[idx] = c - 32;
    SetBit(changed, idx);
    if (invert) {
      colors[idx] = ((screen_bg_color & 0xf) << 4) | (screen_fg_color & 0xf);
    } else {
      colors[idx] = ((screen_fg_color & 0xf) << 4) | (screen_bg_color & 0xf);
    }
  }
}

void chargfx_print(const char *str, bool invert) {
  char c;
  while ((c = *str++)) {
    chargfx_putc(c, invert);
  }
}

void chargfx_write(const char *str, int len, bool invert) {
  for (int i = 0; i < len; i++) {
    chargfx_putc(*str++, invert);
  }
}

void chargfx_draw_region(uint8_t x, uint8_t y, uint8_t width, uint8_t height) {

  int remainder = height;
  while (remainder) {
    int sub_height = (remainder > BUFFER_CHARS) ? BUFFER_CHARS : remainder;
    int sub_y = y + height - remainder;
    remainder -= sub_height;
    chargfx_draw_sub_region(x, sub_y, width, sub_height);
  }
}

// NOTE: we make life easier for ourselves by using the LCD controllers
// orientation command to let us treat the x,y coords passed into this function
// as the visual x & y instead of trying to transform them to the LCDs physical
// x,y coords to compensate for the fact that on the picoTracker the screen is
// mounted rotated 90deg clockwise, ie. the "bottom" of the LCD with the flex
// pcb connector is actually on the left instead of its normal orientation of
// being mounted on the bottom of the LCD
void chargfx_fill_rect(uint8_t color_index, uint16_t x, uint16_t y,
                       uint16_t width, uint16_t height) {
  // Get the RGB565 color from the current foreground palette index
  uint16_t color = palette[color_index];

  // Clip the rectangle to the screen dimensions
  if (x >= ILI9341_TFTHEIGHT || y >= ILI9341_TFTWIDTH) {
    return;
  }
  if (x + width > ILI9341_TFTHEIGHT) {
    width = ILI9341_TFTHEIGHT;
  }
  if (y + height > ILI9341_TFTWIDTH) {
    height = ILI9341_TFTWIDTH;
  }

#ifdef PC_BUILD
  // For PC build, we map x directly to display_x (0,0 is top left)
  uint16_t display_x = x;
  uint16_t display_y = y;
  uint16_t display_w = width;
  uint16_t display_h = height;
#else
  // display_x is from right hand edge and since the picoTracker LCD is mounted
  // rotated 90deg clockwise, the LCDs "physical height" is actually visually
  // speaking the width
  uint16_t display_x = ILI9341_TFTHEIGHT - x - width;
  uint16_t display_y = y;
  uint16_t display_w = width;
  uint16_t display_h = height;
#endif

#ifndef PC_BUILD
  // Set rotation for rectangle drawing
  ili9341_set_command(ILI9341_MADCTL);
  ili9341_command_param(0x28); // 90-degree clockwise rotation
#endif

  // Set display window
  ili9341_set_command(ILI9341_CASET);
  ili9341_command_param16(display_x);
  ili9341_command_param16(display_x + display_w - 1);

  ili9341_set_command(ILI9341_PASET);
  ili9341_command_param16(display_y);
  ili9341_command_param16(display_y + display_h - 1);

  ili9341_set_command(ILI9341_RAMWR);
  ili9341_start_writing();

  // just use the char cell buffer for our line buffer as its more than big
  // enough
  for (uint16_t i = 0; i < display_w; i++) {
    buffer[i] = color;
  }

  // Write the buffer for each column
  for (uint16_t i = 0; i < display_h; i++) {
    ili9341_write_data_continuous(buffer, display_w * sizeof(uint16_t));
  }

  ili9341_stop_writing();

  // Restore original rotation
  ili9341_set_command(ILI9341_MADCTL);
  ili9341_command_param(0xC0);
}

inline void chargfx_draw_sub_region(uint8_t x, uint8_t y, uint8_t width,
                                    uint8_t height) {
  assert(height <= BUFFER_CHARS);

#ifdef PC_BUILD
  // Standard Row-Major Rendering for PC Screen
  uint16_t screen_x = x * CHAR_WIDTH;
  uint16_t screen_y = y * CHAR_HEIGHT;
  uint16_t screen_width = width * CHAR_WIDTH;
  uint16_t screen_height = height * CHAR_HEIGHT;

  // Set Window (X, Y, Width, Height)
  ili9341_set_command(ILI9341_CASET);
  ili9341_command_param16(screen_x);
  ili9341_command_param16(screen_x + screen_width - 1);

  ili9341_set_command(ILI9341_PASET);
  ili9341_command_param16(screen_y);
  ili9341_command_param16(screen_y + screen_height - 1);

  ili9341_set_command(ILI9341_RAMWR);
  ili9341_start_writing();

  const font_t *font = fonts[ui_font_index];

  // Fill buffer row by row
  // We process 'height' rows of characters
  for (int char_row = 0; char_row < height; char_row++) {
      // For each pixel row within the character height (8 pixels)
      for (int py = 0; py < CHAR_HEIGHT; py++) {
          uint16_t *buffer_idx = buffer;
          // Process 'width' columns of characters
          for (int char_col = 0; char_col < width; char_col++) {
              int16_t idx = (y + char_row) * TEXT_WIDTH + (x + char_col);
              uint8_t character = screen[idx];
              uint16_t fg_color = palette[colors[idx] >> 4];
              uint16_t bg_color = palette[colors[idx] & 0xf];
              
              const uint16_t *pixel_data = (*font)[character];
              uint16_t pix = pixel_data[CHAR_HEIGHT - 1 - py]; // Font data might need careful indexing

              // Loop through pixels in the character row (CHAR_WIDTH)
              // Font is defined as 8x8, usually array of 8 shorts?
              // Font definition: uint16_t [8] -> each uint16_t is a column?
              // Let's check font.h format.
              // Assuming font is: array of 8 uint16_t, where each uint16_t is a column (top to bottom LSB to MSB or vice versa)
              // The original code:
              //   uint16_t pix = pixel_data[j]; (j is y loop 0..7)
              //   (pix & mask) -> mask based on x bit
              // This implies pixel_data[j] is the j-th row? NO.
              // Original loop:
              //   pixel_data[j] where j is 7 down to 0. (Rows)
              //   mask = 1 << (7-bit) (Columns)
              //   So pixel_data is array of 8 rows?
              
              // Let's assume pixel_data[py] is the row data.
              uint16_t row_data = pixel_data[CHAR_HEIGHT - 1 - py]; 
              
              for (int px = 0; px < CHAR_WIDTH; px++) {
                   uint16_t mask = 1 << (CHAR_WIDTH - 1 - px);
                   // Wait, checking original again:
                   /*
                   const uint16_t *pixel_data = (*font)[character];
                     for (int j = CHAR_HEIGHT - 1; j >= 0; j--) {
                       uint16_t pix = pixel_data[j];
                       *buffer_idx++ = (pix & mask) ? fg_color : bg_color;
                     }
                   */
                   // This original loop writes a vertical column of pixels for the character.
                   // The outer loops iterate X (page), then bit (pixel column in char), then Y (char row).
                   // So it writes a vertical stripe of pixels.
                   
                   // For normal raster:
                   // We need to write Row 0 of all chars in the line.
                   // Then Row 1...
              }
          }
       }
   }
   
   // RESTART STRATEGY:
   // I need to confirm Font Format.
   // Original code:
   // for bit (column 7..0)
   //   u16 mask = 1 << (7-bit)
   //   for col (char rows Y)
   //     pixel_data = font[char]
   //     for j (pixel y 7..0)
   //        pix = pixel_data[j]
   //        write (pix & mask)
   
   // This implies pixel_data[j] represents a ROW of the character.
   // And consistent with standard bitmap fonts.
   // So pixel_data[0] is top row, pixel_data[7] is bottom row.
   
   // So for Row-Major render:
   for (int char_row = 0; char_row < height; char_row++) {
      int screen_char_y = y + char_row;
      for (int py = 0; py < CHAR_HEIGHT; py++) {
           int font_row_idx = CHAR_HEIGHT - 1 - py; // Original loop does j=7..0 so bottom-up?
           // Original loop: for j=7..0; pix = pixel_data[j].
           // It writes to buffer sequentially.
           // Since original writes columns, and iterates j=7..0, it writes bottom-to-top of the column?
           // Or is it writing to a buffer that is transposed?
           
           // If the screen is rotated 90 deg clockwise:
           // Physical Top-Right is Logical Top-Left.
           // Writes go "Down" the physical screen (Logical Right).
           
           // Use simpler logic: Just Draw pixels 1:1.
           // I'll re-implement the loop to be standard:
           // line buffer for one raster row (width * CHAR_WIDTH pixels)
           uint16_t *line_buffer = buffer; // reuse existing buffer
           
           for (int char_col = 0; char_col < width; char_col++) {
               int idx = screen_char_y * TEXT_WIDTH + (x + char_col);
               uint8_t character = screen[idx];
               uint16_t fg_color = palette[colors[idx] >> 4];
               uint16_t bg_color = palette[colors[idx] & 0xf];
               const uint16_t *pixel_data = (*font)[character];
               
               // Get the row of pixels for this scanline 'py'
               // Original used j=7..0. If CHAR_HEIGHT=8.
               // Let's try simplified indexing: pixel_data[py]
               // But original loop order suggests 7 is top? or bottom?
               // If char looks upright, and it renders j=7 first, and fills buffer...
               
               // Let's assume standard font encoding: index 0 is top row.
               // Original code:
               //   for j = 7 down to 0:
               //      write pixel
               // This implies it writes 7 (Bottom?) then 6... then 0 (Top).
               // So it writes Bottom-to-Top?
               
               uint16_t pixels = pixel_data[CHAR_HEIGHT - 1 - py]; // Try this
               
               for (int bit = 0; bit < CHAR_WIDTH; bit++) {
                   // MSB left? 1 << (7-bit)
                   uint16_t mask = 1 << (CHAR_WIDTH - 1 - bit);
                   *line_buffer++ = (pixels & mask) ? fg_color : bg_color;
               }
           }
           ili9341_write_data_continuous(buffer, width * CHAR_WIDTH * sizeof(uint16_t));
           line_buffer = buffer; // Reset for next scanline
      }
   }

#else
  uint16_t screen_x = x * CHAR_WIDTH;
  uint16_t screen_y = (TEXT_HEIGHT - height - y) * CHAR_HEIGHT;
  uint16_t screen_width = width * CHAR_WIDTH;
  uint16_t screen_height = height * CHAR_HEIGHT;

  // column address set
  ili9341_set_command(ILI9341_CASET);
  ili9341_command_param16(screen_y);
  ili9341_command_param16(screen_y + screen_height - 1);

  // page address set
  ili9341_set_command(ILI9341_PASET);
  ili9341_command_param16(screen_x);
  ili9341_command_param16(screen_x + screen_width - 1);
  // start writing
  ili9341_set_command(ILI9341_RAMWR);

  ili9341_start_writing();

  const font_t *font = fonts[ui_font_index];

  for (int page = x; page < x + width; page++) {
    // create one column of screen information
    uint16_t *buffer_idx = buffer;

    // Iterate over columns of bits (CHAR_WIDTH)
    for (int bit = CHAR_WIDTH - 1; bit >= 0; bit--) {
      uint16_t mask = 1 << (CHAR_WIDTH - 1 - bit);
      // Iterate over characters in column (Y)
      for (int col = y + height - 1; col >= y; col--) {
        int16_t idx = col * TEXT_WIDTH + page;
        uint8_t character = screen[idx];
        uint16_t fg_color = palette[colors[idx] >> 4];
        uint16_t bg_color = palette[colors[idx] & 0xf];

        const uint16_t *pixel_data = (*font)[character];

        // draw the character into the buffer
        for (int j = CHAR_HEIGHT - 1; j >= 0; j--) {
          uint16_t pix = pixel_data[j];
          *buffer_idx++ = (pix & mask) ? fg_color : bg_color;
        }
      }
    }
    ili9341_write_data_continuous(buffer,
                                  CHAR_WIDTH * screen_height * sizeof(int16_t));
  }
#endif
  ili9341_stop_writing();
}

void chargfx_draw_changed() {
  for (int idx = 0; idx < TEXT_HEIGHT * TEXT_WIDTH; idx++) {
    if (TestBit(changed, idx)) {
      ClearBit(changed, idx);
      // check adjacent in order to find bigger rectangle
      uint16_t y = idx / TEXT_WIDTH;
      uint16_t x = idx - (TEXT_WIDTH * y);

      int height = 1;
      // first pass tests the height
      for (int probe_y = y + 1; probe_y < TEXT_HEIGHT; probe_y++) {
        int probe_idx = probe_y * TEXT_WIDTH + x;
        if (TestBit(changed, probe_idx)) {
          ClearBit(changed, probe_idx);
          height++;
          continue;
        }
        break;
      }

      int16_t width = 1;
      // having the height, we can test every subsequent column
      for (int probe_x = x + 1; probe_x < TEXT_WIDTH; probe_x++) {
        for (int probe_y = y; probe_y < y + height; probe_y++) {
          // if we don't get to max height, then abort
          int probe_idx = probe_y * TEXT_WIDTH + probe_x;
          if (!TestBit(changed, probe_idx)) {
            // undo last column
            for (int undo_y = y; undo_y < probe_y; undo_y++) {
              SetBit(changed, undo_y * TEXT_WIDTH + probe_x);
            }
            goto end;
          }
          ClearBit(changed, probe_idx);
        }
        width++;
      }
    end:
      chargfx_draw_region(x, y, width, height);
    }
  }
}

void chargfx_draw_changed_simple() {
  // This method is better (faster) for fewer characters changed
  for (int idx = 0; idx < TEXT_HEIGHT * TEXT_WIDTH; idx++) {
    if (TestBit(changed, idx)) {
      ClearBit(changed, idx);
      uint16_t y = idx / TEXT_WIDTH;
      uint16_t x = idx - (TEXT_WIDTH * y);
      chargfx_draw_region(x, y, 1, 1);
    }
  }
}

void chargfx_draw_screen() {
  // draw the whole screen
  chargfx_draw_region(0, 0, TEXT_WIDTH, TEXT_HEIGHT);
}

void chargfx_set_palette_color(int idx, uint16_t rgb565_color) {
  palette[idx] = SWAP_BYTES(rgb565_color);
}

void chargfx_init() { ili9341_init(); }
