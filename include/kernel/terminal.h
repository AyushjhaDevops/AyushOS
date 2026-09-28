#ifndef AYUSHOS_TERMINAL_H
#define AYUSHOS_TERMINAL_H

#include <stdint.h>

#define VGA_WIDTH  80
#define VGA_HEIGHT 25

typedef enum
{
    TERMINAL_COLOR_BLACK = 0,
    TERMINAL_COLOR_BLUE = 1,
    TERMINAL_COLOR_GREEN = 2,
    TERMINAL_COLOR_CYAN = 3,
    TERMINAL_COLOR_RED = 4,
    TERMINAL_COLOR_MAGENTA = 5,
    TERMINAL_COLOR_BROWN = 6,
    TERMINAL_COLOR_LIGHT_GREY = 7,
    TERMINAL_COLOR_DARK_GREY = 8,
    TERMINAL_COLOR_LIGHT_BLUE = 9,
    TERMINAL_COLOR_LIGHT_GREEN = 10,
    TERMINAL_COLOR_LIGHT_CYAN = 11,
    TERMINAL_COLOR_LIGHT_RED = 12,
    TERMINAL_COLOR_LIGHT_MAGENTA = 13,
    TERMINAL_COLOR_LIGHT_BROWN = 14,
    TERMINAL_COLOR_WHITE = 15
} terminal_color_t;

void terminal_initialize(void);

void terminal_clear(void);

void terminal_putchar(char c);

void terminal_write(const char *data);

void terminal_write_string(const char *data);

void terminal_set_color(uint8_t color);

void terminal_set_cursor(uint8_t x, uint8_t y);

#endif
