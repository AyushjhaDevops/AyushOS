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

    klog_ok(
        "IRQ handler table initialized"
    );
}

void irq_register_handler(
    uint8_t irq,
    irq_handler_t handler
)
{
    if (irq >= IRQ_COUNT)
    {
        return;
    }

    irq_handlers[irq] = handler;
}

void irq_handler(irq_frame_t *frame)
{
    if (frame == 0)
    {
        return;
    }

    /*
     * Hardware IRQ vector:
     *
     * IRQ0  -> vector 32
     * IRQ1  -> vector 33
     * ...
     * IRQ15 -> vector 47
     *
     * Therefore:
     *
     * IRQ = vector - 32
     */
    if (frame->int_no < 32)
    {
        return;
    }

    uint8_t irq =
        (uint8_t)(frame->int_no - 32);

    if (irq >= IRQ_COUNT)
    {
        return;
    }

    /*
     * Call registered device handler.
     */
    if (irq_handlers[irq] != 0)
    {
        irq_handlers[irq](frame);
    }

    /*
     * Tell the PIC that the interrupt
     * has been completely handled.
     */
    pic_send_eoi(irq);
}
