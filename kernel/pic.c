#include <stdint.h>

#include <kernel/pic.h>
#include <kernel/io.h>

#define PIC_ICW1_INIT 0x10
#define PIC_ICW1_ICW4 0x01

#define PIC_ICW4_8086 0x01

void pic_initialize(void)
{
    /*
     * Save current interrupt masks.
     */
    uint8_t master_mask = inb(PIC1_DATA);
    uint8_t slave_mask  = inb(PIC2_DATA);

    /*
     * Begin PIC initialization.
     *
     * ICW1:
     * 0x10 = initialization
     * 0x01 = ICW4 will be sent
     */
    outb(
        PIC1_COMMAND,
        PIC_ICW1_INIT | PIC_ICW1_ICW4
    );

    outb(
        PIC2_COMMAND,
        PIC_ICW1_INIT | PIC_ICW1_ICW4
    );

    /*
     * ICW2
     *
     * Master:
     * IRQ0-IRQ7 -> vectors 32-39
     */
    outb(
        PIC1_DATA,
        IRQ_BASE
    );

    /*
     * Slave:
     * IRQ8-IRQ15 -> vectors 40-47
     */
    outb(
        PIC2_DATA,
        IRQ_BASE + 8
    );

    /*
     * ICW3
     *
     * Master IRQ2 has the slave PIC connected.
     */
    outb(
        PIC1_DATA,
        0x04
    );

    /*
     * Slave is connected to master IRQ2.
     */
    outb(
        PIC2_DATA,
        0x02
    );

    /*
     * ICW4
     *
     * 8086/88 mode.
     */
    outb(
        PIC1_DATA,
        PIC_ICW4_8086
    );

    outb(
        PIC2_DATA,
        PIC_ICW4_8086
    );

    /*
     * Restore the original masks.
     */
    outb(
        PIC1_DATA,
        master_mask
    );

    outb(
        PIC2_DATA,
        slave_mask
    );
}

void pic_send_eoi(uint8_t irq)
{
    /*
     * IRQ8-IRQ15 originate from the slave PIC,
     * therefore the slave must receive EOI first.
     */
    if (irq >= 8)
    {
        outb(
            PIC2_COMMAND,
            PIC_EOI
        );
    }

    /*
     * Master PIC always receives EOI.
     */
    outb(
        PIC1_COMMAND,
        PIC_EOI
    );
}

void pic_set_mask(uint8_t irq)
{
    uint16_t port;

    if (irq < 8)
    {
        port = PIC1_DATA;
    }
    else
    {
        irq -= 8;
        port = PIC2_DATA;
    }

    uint8_t mask = inb(port);

    mask |= (uint8_t)(1U << irq);

    outb(
        port,
        mask
    );
}

void pic_clear_mask(uint8_t irq)
{
    uint16_t port;

    if (irq < 8)
    {
        port = PIC1_DATA;
    }
    else
    {
        irq -= 8;
        port = PIC2_DATA;
    }

    uint8_t mask = inb(port);

    mask &= (uint8_t)~(1U << irq);

    outb(
        port,
        mask
    );
}
