#ifndef AYUSHOS_IRQ_H
#define AYUSHOS_IRQ_H

#include <stdint.h>

#define IRQ_COUNT 16

/*
 * Interrupt frame for kernel-mode IRQs.
 *
 * AyushOS currently runs everything in ring 0, so the CPU does
 * NOT push useresp/ss when entering an interrupt.
 *
 * Stack layout:
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
 * ERR_CODE
 * EIP
 * CS
 * EFLAGS
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

} irq_frame_t;

/*
 * IRQ handlers can return a different frame.
 *
 * This is important for preemptive scheduling:
 *
 * current process frame
 *        ↓
 * scheduler chooses another process
 *        ↓
 * return next process frame
 *        ↓
 * IRQ assembly restores next process
 */
typedef irq_frame_t *(*irq_handler_t)(irq_frame_t *frame);

void irq_initialize(void);

void irq_register_handler(
    uint8_t irq,
    irq_handler_t handler
);

irq_frame_t *irq_handler(
    irq_frame_t *frame
);

#endif
