#ifndef AYUSHOS_TIMER_H
#define AYUSHOS_TIMER_H

#include <stdint.h>

/*
 * PIT input clock frequency.
 *
 * The traditional PIT clock runs at approximately
 * 1.193182 MHz.
 */
#define PIT_BASE_FREQUENCY 1193182U

/*
 * PIT interrupt frequency.
 *
 * 100 Hz means:
 *
 *     100 interrupts / second
 *
 * therefore:
 *
 *     1 tick = 10 ms
 */
#define PIT_FREQUENCY 100U

/*
 * Initialize the PIT timer.
 */
void timer_initialize(void);

/*
 * Return number of timer ticks since initialization.
 */
uint32_t timer_get_ticks(void);

/*
 * Return configured timer frequency.
 */
uint32_t timer_get_frequency(void);

/*
 * Return uptime in milliseconds.
 */
uint32_t timer_get_milliseconds(void);

/*
 * Busy-wait delay.
 *
 * The CPU remains active while waiting.
 *
 * Useful for very small delays and hardware timing.
 */
void timer_delay_ms(uint32_t milliseconds);

/*
 * Interrupt-friendly sleep.
 *
 * The CPU executes HLT while waiting for timer interrupts.
 *
 * This does not yet block a process because process
 * management does not exist until a later phase.
 */
void timer_sleep_ms(uint32_t milliseconds);

#endif
