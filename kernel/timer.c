#include <stdint.h>

#include <kernel/timer.h>
#include <kernel/io.h>
#include <kernel/irq.h>
#include <kernel/pic.h>
#include <kernel/kprintf.h>
#include <kernel/process.h>


#define PIT_COMMAND   0x43
#define PIT_CHANNEL0  0x40

/*
 * Original Intel 8253/8254 PIT input clock.
 */
#define PIT_BASE_FREQUENCY 1193182U

/*
 * AyushOS scheduler timer frequency.
 */
#define TIMER_FREQUENCY 100U

#define PIT_DIVISOR \
    (PIT_BASE_FREQUENCY / TIMER_FREQUENCY)


static volatile uint32_t timer_ticks = 0;


/* ============================================================
 * Timer IRQ handler
 * ============================================================ */

static irq_frame_t *timer_irq_handler(
    irq_frame_t *frame)
{
    /*
     * One PIT interrupt = one scheduler tick.
     */
    timer_ticks++;

    /*
     * Account for CPU time and allow the scheduler
     * to select another process.
     */
    return process_schedule_tick(frame);
}


/* ============================================================
 * Timer initialization
 * ============================================================ */

void timer_initialize(void)
{
    uint16_t divisor =
        (uint16_t)PIT_DIVISOR;


    /*
     * Channel 0
     *
     * 0x36 =
     *
     * 00      channel 0
     * 11      access low byte + high byte
     * 011     mode 3
     * 0       binary mode
     */
    outb(
        PIT_COMMAND,
        0x36
    );


    /*
     * Send divisor low byte.
     */
    outb(
        PIT_CHANNEL0,
        (uint8_t)(divisor & 0xFF)
    );


    /*
     * Send divisor high byte.
     */
    outb(
        PIT_CHANNEL0,
        (uint8_t)((divisor >> 8) & 0xFF)
    );


    timer_ticks = 0;


    /*
     * Register IRQ0.
     */
    irq_register_handler(
        0,
        timer_irq_handler
    );


    /*
     * Unmask IRQ0.
     */
    pic_clear_mask(0);


    kprintf(
        "[TIMER] PIT base frequency: %u Hz\n",
        PIT_BASE_FREQUENCY
    );

    kprintf(
        "[TIMER] Timer frequency: %u Hz\n",
        TIMER_FREQUENCY
    );

    kprintf(
        "[TIMER] PIT divisor: %u\n",
        (uint32_t)divisor
    );

    kprintf(
        "[TIMER] Scheduler quantum: %u ticks\n",
        process_get_quantum_ticks()
    );
}


/* ============================================================
 * Timer API
 * ============================================================ */

uint32_t timer_get_ticks(void)
{
    return timer_ticks;
}


uint32_t timer_get_milliseconds(void)
{
    return
        timer_ticks *
        (1000U / TIMER_FREQUENCY);
}


void timer_sleep(
    uint32_t milliseconds)
{
    uint32_t start =
        timer_get_milliseconds();

    while (
        timer_get_milliseconds() -
        start <
        milliseconds
    )
    {
        __asm__ volatile ("hlt");
    }
}
