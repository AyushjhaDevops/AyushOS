#include <stdint.h>

#include <kernel/irq.h>
#include <kernel/pic.h>
#include <kernel/kprintf.h>

static irq_handler_t irq_handlers[IRQ_COUNT];

void irq_initialize(void)
{
    for (uint32_t i = 0; i < IRQ_COUNT; i++)
    {
        irq_handlers[i] = 0;
    }

    klog_ok("IRQ handler table initialized");
}

void irq_register_handler(
    uint8_t irq,
    irq_handler_t handler)
{
    if (irq >= IRQ_COUNT)
    {
        return;
    }

    irq_handlers[irq] = handler;
}

/*
 * Main IRQ dispatcher.
 *
 * The handler may return the same frame or a completely different
 * process frame.
 */
irq_frame_t *irq_handler(irq_frame_t *frame)
{
    if (frame == 0)
    {
        return frame;
    }

    /*
     * Hardware IRQ vectors start at 32.
     */
    if (frame->int_no < 32)
    {
        return frame;
    }

    uint8_t irq =
        (uint8_t)(frame->int_no - 32);

    if (irq >= IRQ_COUNT)
    {
        return frame;
    }

    /*
     * Execute registered handler.
     *
     * A timer handler may return a different frame because the
     * scheduler may have selected another process.
     */
    if (irq_handlers[irq] != 0)
    {
        frame = irq_handlers[irq](frame);
    }

    /*
     * Acknowledge the interrupt after the handler has completed.
     */
    pic_send_eoi(irq);

    return frame;
}
