#ifndef AYUSHOS_KEYBOARD_H
#define AYUSHOS_KEYBOARD_H

#include <stdint.h>

/*
 * Initialize the PS/2 keyboard subsystem.
 */
void keyboard_initialize(void);

/*
 * Poll the PS/2 controller and process any
 * available keyboard scancodes.
 *
 * This is the Phase 3 polling interface.
 */
void keyboard_poll(void);

/*
 * Return non-zero if a character is available
 * in the keyboard buffer.
 */
int keyboard_has_char(void);

/*
 * Get one character from the keyboard buffer.
 *
 * Returns:
 *   - ASCII character when available
 *   - 0 when no character is available
 */
char keyboard_getchar(void);

/*
 * Return the number of characters currently
 * stored in the keyboard buffer.
 */
uint32_t keyboard_buffer_count(void);

#endif
