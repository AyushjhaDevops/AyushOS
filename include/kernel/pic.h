#ifndef AYUSHOS_PIC_H
#define AYUSHOS_PIC_H

#include <stdint.h>

/*
 * Master PIC
 */
#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21

/*
 * Slave PIC
 */
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

/*
 * End Of Interrupt command.
 */
#define PIC_EOI 0x20

/*
 * Hardware IRQs will be mapped to
 * IDT vectors 32-47.
 */
#define IRQ_BASE 32

void pic_initialize(void);

void pic_send_eoi(uint8_t irq);

void pic_set_mask(uint8_t irq);

void pic_clear_mask(uint8_t irq);

#endif
