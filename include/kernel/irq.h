#ifndef AYUSHOS_IRQ_H
#define AYUSHOS_IRQ_H

#include <stdint.h>

#define IRQ_COUNT 16

/*
 * Stack frame created by irq.asm.
 *
 * Layout:
 *
 * DS
 * EDI
 * ESI
 * EBP
 * ESP
 * EBX
 * EDX
 * ECX
 * EAX
 * INT_NO
 * ERROR_CODE
 * EIP
 * CS
 * EFLAGS
 * USERESP
 * SS
 */
typedef struct __attribute__((packed))
{
    uint32_t ds;

    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t int_no;
    uint32_t err_code;

    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t useresp;
    uint32_t ss;

} irq_frame_t;

/*
 * IRQ handler function type.
 */
typedef void (*irq_handler_t)(irq_frame_t *frame);

/*
 * Initialize IRQ handler table.
 */
void irq_initialize(void);

/*
 * Register a handler for IRQ 0-15.
 */
void irq_register_handler(
    uint8_t irq,
    irq_handler_t handler
);

/*
 * Main IRQ dispatcher.
 */
void irq_handler(irq_frame_t *frame);

#endif
