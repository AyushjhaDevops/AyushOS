#include <stdint.h>

#include <kernel/io.h>
#include <kernel/keyboard.h>

#define PS2_DATA_PORT       0x60
#define PS2_STATUS_PORT     0x64

#define PS2_STATUS_OUTPUT_FULL 0x01

#define KEYBOARD_BUFFER_SIZE 128

/*
 * Keyboard circular buffer.
 */
static volatile char keyboard_buffer[KEYBOARD_BUFFER_SIZE];

static volatile uint32_t keyboard_buffer_head;
static volatile uint32_t keyboard_buffer_tail;
static volatile uint32_t keyboard_buffer_size;

/*
 * Shift state.
 */
static uint8_t left_shift_pressed;
static uint8_t right_shift_pressed;

/*
 * Scancode Set 1 - unshifted ASCII.
 *
 * Index = keyboard scancode.
 */
static const char scancode_to_ascii[128] =
{
    [0x01] = 27,    /* ESC */

    [0x02] = '1',
    [0x03] = '2',
    [0x04] = '3',
    [0x05] = '4',
    [0x06] = '5',
    [0x07] = '6',
    [0x08] = '7',
    [0x09] = '8',
    [0x0A] = '9',
    [0x0B] = '0',

    [0x0C] = '-',
    [0x0D] = '=',
    [0x0E] = '\b',
    [0x0F] = '\t',

    [0x10] = 'q',
    [0x11] = 'w',
    [0x12] = 'e',
    [0x13] = 'r',
    [0x14] = 't',
    [0x15] = 'y',
    [0x16] = 'u',
    [0x17] = 'i',
    [0x18] = 'o',
    [0x19] = 'p',

    [0x1A] = '[',
    [0x1B] = ']',
    [0x1C] = '\n',

    [0x1E] = 'a',
    [0x1F] = 's',
    [0x20] = 'd',
    [0x21] = 'f',
    [0x22] = 'g',
    [0x23] = 'h',
    [0x24] = 'j',
    [0x25] = 'k',
    [0x26] = 'l',

    [0x27] = ';',
    [0x28] = '\'',
    [0x29] = '`',

    [0x2B] = '\\',

    [0x2C] = 'z',
    [0x2D] = 'x',
    [0x2E] = 'c',
    [0x2F] = 'v',
    [0x30] = 'b',
    [0x31] = 'n',
    [0x32] = 'm',

    [0x33] = ',',
    [0x34] = '.',
    [0x35] = '/',
    [0x39] = ' '
};

/*
 * Shifted characters.
 */
static const char scancode_to_ascii_shift[128] =
{
    [0x01] = 27,

    [0x02] = '!',
    [0x03] = '@',
    [0x04] = '#',
    [0x05] = '$',
    [0x06] = '%',
    [0x07] = '^',
    [0x08] = '&',
    [0x09] = '*',
    [0x0A] = '(',
    [0x0B] = ')',

    [0x0C] = '_',
    [0x0D] = '+',
    [0x0E] = '\b',
    [0x0F] = '\t',

    [0x10] = 'Q',
    [0x11] = 'W',
    [0x12] = 'E',
    [0x13] = 'R',
    [0x14] = 'T',
    [0x15] = 'Y',
    [0x16] = 'U',
    [0x17] = 'I',
    [0x18] = 'O',
    [0x19] = 'P',

    [0x1A] = '{',
    [0x1B] = '}',
    [0x1C] = '\n',

    [0x1E] = 'A',
    [0x1F] = 'S',
    [0x20] = 'D',
    [0x21] = 'F',
    [0x22] = 'G',
    [0x23] = 'H',
    [0x24] = 'J',
    [0x25] = 'K',
    [0x26] = 'L',

    [0x27] = ':',
    [0x28] = '"',
    [0x29] = '~',

    [0x2B] = '|',

    [0x2C] = 'Z',
    [0x2D] = 'X',
    [0x2E] = 'C',
    [0x2F] = 'V',
    [0x30] = 'B',
    [0x31] = 'N',
    [0x32] = 'M',

    [0x33] = '<',
    [0x34] = '>',
    [0x35] = '?',
    [0x39] = ' '
};

/*
 * Add a character to the circular buffer.
 */
static void keyboard_buffer_push(char character)
{
    /*
     * Drop the character if the buffer is full.
     */
    if (keyboard_buffer_size >= KEYBOARD_BUFFER_SIZE)
    {
        return;
    }

    keyboard_buffer[keyboard_buffer_head] = character;

    keyboard_buffer_head++;

    if (keyboard_buffer_head >= KEYBOARD_BUFFER_SIZE)
    {
        keyboard_buffer_head = 0;
    }

    keyboard_buffer_size++;
}

/*
 * Remove a character from the circular buffer.
 */
static char keyboard_buffer_pop(void)
{
    char character;

    if (keyboard_buffer_size == 0)
    {
        return 0;
    }

    character = keyboard_buffer[keyboard_buffer_tail];

    keyboard_buffer_tail++;

    if (keyboard_buffer_tail >= KEYBOARD_BUFFER_SIZE)
    {
        keyboard_buffer_tail = 0;
    }

    keyboard_buffer_size--;

    return character;
}

/*
 * Decode one Set 1 scancode.
 */
static void keyboard_process_scancode(uint8_t scancode)
{
    /*
     * Key release.
     *
     * In Set 1, a release event is the make code
     * with bit 7 set.
     */
    if (scancode & 0x80)
    {
        uint8_t make_code = scancode & 0x7F;

        if (make_code == 0x2A)
        {
            left_shift_pressed = 0;
        }
        else if (make_code == 0x36)
        {
            right_shift_pressed = 0;
        }

        return;
    }

    /*
     * Shift key press.
     */
    if (scancode == 0x2A)
    {
        left_shift_pressed = 1;
        return;
    }

    if (scancode == 0x36)
    {
        right_shift_pressed = 1;
        return;
    }

    /*
     * Ignore scancodes outside our table.
     */
    if (scancode >= 128)
    {
        return;
    }

    char character;

    if (left_shift_pressed || right_shift_pressed)
    {
        character = scancode_to_ascii_shift[scancode];
    }
    else
    {
        character = scancode_to_ascii[scancode];
    }

    /*
     * Only put recognized characters into the buffer.
     */
    if (character != 0)
    {
        keyboard_buffer_push(character);
    }
}

void keyboard_initialize(void)
{
    keyboard_buffer_head = 0;
    keyboard_buffer_tail = 0;
    keyboard_buffer_size = 0;

    left_shift_pressed = 0;
    right_shift_pressed = 0;

    /*
     * Drain any old data currently waiting in the
     * PS/2 output buffer.
     */
    while (inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_FULL)
    {
        (void)inb(PS2_DATA_PORT);
    }
}

void keyboard_poll(void)
{
    /*
     * Check whether the PS/2 controller has a byte
     * available.
     */
    while (inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_FULL)
    {
        uint8_t scancode = inb(PS2_DATA_PORT);

        keyboard_process_scancode(scancode);
    }
}

int keyboard_has_char(void)
{
    return keyboard_buffer_size != 0;
}

char keyboard_getchar(void)
{
    return keyboard_buffer_pop();
}

uint32_t keyboard_buffer_count(void)
{
    return keyboard_buffer_size;
}
