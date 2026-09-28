#include <stdint.h>
#include <kernel/terminal.h>

static volatile uint16_t *const VGA_MEMORY =
    (volatile uint16_t *)0xB8000;

static uint8_t terminal_row;
static uint8_t terminal_column;
static uint8_t terminal_color;

static uint16_t terminal_entry(
    unsigned char character,
    uint8_t color)
{
    return ((uint16_t)color << 8) | character;
}

void terminal_initialize(void)
{
    terminal_row = 0;
    terminal_column = 0;

    terminal_color =
        ((uint8_t)TERMINAL_COLOR_BLACK << 4) |
        (uint8_t)TERMINAL_COLOR_LIGHT_GREY;

    terminal_clear();
}

void terminal_clear(void)
{
    for (uint32_t y = 0; y < VGA_HEIGHT; y++)
    {
        for (uint32_t x = 0; x < VGA_WIDTH; x++)
        {
            const uint32_t index =
                y * VGA_WIDTH + x;

            VGA_MEMORY[index] =
                terminal_entry(' ', terminal_color);
        }
    }

    terminal_row = 0;
    terminal_column = 0;
}

void terminal_set_color(uint8_t color)
{
    terminal_color = color;
}

void terminal_set_cursor(uint8_t x, uint8_t y)
{
    if (x >= VGA_WIDTH)
        x = VGA_WIDTH - 1;

    if (y >= VGA_HEIGHT)
        y = VGA_HEIGHT - 1;

    terminal_column = x;
    terminal_row = y;
}

static void terminal_newline(void)
{
    terminal_column = 0;

    if (terminal_row < VGA_HEIGHT - 1)
    {
        terminal_row++;
        return;
    }

    /*
     * Temporary behavior for Phase 2:
     * wrap back to the top when the screen is full.
     *
     * A proper scrolling implementation will be
     * introduced later.
     */
    terminal_row = 0;
    terminal_clear();
}

void terminal_putchar(char c)
{
    if (c == '\n')
    {
        terminal_newline();
        return;
    }

    if (c == '\r')
    {
        terminal_column = 0;
        return;
    }

    if (c == '\t')
    {
        terminal_column =
            (terminal_column + 4) & ~3;

        if (terminal_column >= VGA_WIDTH)
            terminal_newline();

        return;
    }

    const uint32_t index =
        terminal_row * VGA_WIDTH +
        terminal_column;

    VGA_MEMORY[index] =
        terminal_entry(
            (unsigned char)c,
            terminal_color);

    terminal_column++;

    if (terminal_column >= VGA_WIDTH)
    {
        terminal_newline();
    }
}

void terminal_write(const char *data)
{
    if (data == 0)
        return;

    for (uint32_t i = 0; data[i] != '\0'; i++)
    {
        terminal_putchar(data[i]);
    }
}

void terminal_write_string(const char *data)
{
    terminal_write(data);
}
