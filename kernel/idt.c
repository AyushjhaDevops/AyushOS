#include <stdint.h>

#include <kernel/idt.h>
#include <kernel/kprintf.h>

#define IDT_ENTRIES 256

#define IDT_PRESENT       0x80
#define IDT_RING0         0x00
#define IDT_INTERRUPT_32  0x0E

typedef struct __attribute__((packed))
{
    uint16_t base_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  flags;
    uint16_t base_high;
} idt_entry_t;

typedef struct __attribute__((packed))
{
    uint16_t limit;
    uint32_t base;
} idt_ptr_t;

typedef struct
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
} interrupt_frame_t;

idt_entry_t idt[IDT_ENTRIES];
idt_ptr_t idt_ptr;

extern void idt_flush(uint32_t idt_ptr_address);

extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);
extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);
static void idt_set_gate(
    uint8_t vector,
    uint32_t base,
    uint16_t selector,
    uint8_t flags)
{
    idt[vector].base_low =
        (uint16_t)(base & 0xFFFF);

    idt[vector].base_high =
        (uint16_t)((base >> 16) & 0xFFFF);

    idt[vector].selector = selector;
    idt[vector].zero = 0;
    idt[vector].flags = flags;
}

void exception_handler(interrupt_frame_t *frame)
{
    static const char *exception_names[32] =
    {
        "Divide Error",
        "Debug",
        "Non-Maskable Interrupt",
        "Breakpoint",
        "Overflow",
        "Bound Range Exceeded",
        "Invalid Opcode",
        "Device Not Available",
        "Double Fault",
        "Coprocessor Segment Overrun",
        "Invalid TSS",
        "Segment Not Present",
        "Stack-Segment Fault",
        "General Protection Fault",
        "Page Fault",
        "Reserved",
        "x87 Floating-Point Exception",
        "Alignment Check",
        "Machine Check",
        "SIMD Floating-Point Exception",
        "Virtualization Exception",
        "Control Protection Exception",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Hypervisor Injection Exception",
        "VMM Communication Exception",
        "Security Exception",
        "Reserved"
    };

    kprintf("\n\n");
    klog_error("CPU EXCEPTION");

    if (frame->int_no < 32)
    {
        kprintf(
            "  Exception : %s\n",
            exception_names[frame->int_no]
        );
    }

    kprintf(
        "  Vector    : %u\n",
        frame->int_no
    );

    kprintf(
        "  Error     : 0x%x\n",
        frame->err_code
    );

    kprintf(
        "  EIP       : 0x%x\n",
        frame->eip
    );

    kprintf(
        "  CS        : 0x%x\n",
        frame->cs
    );

    kprintf(
        "  EFLAGS    : 0x%x\n",
        frame->eflags
    );

    /*
     * Stop execution.
     *
     * Recovery will be added later when the process
     * and kernel isolation architecture exists.
     */
    while (1)
    {
        __asm__ volatile ("cli");
        __asm__ volatile ("hlt");
    }
}

void idt_initialize(void)
{
    idt_ptr.limit =
        sizeof(idt) - 1;

    idt_ptr.base =
        (uint32_t)&idt;

    /*
     * Clear all IDT entries.
     */
    for (uint32_t i = 0; i < IDT_ENTRIES; i++)
    {
        idt[i].base_low = 0;
        idt[i].base_high = 0;
        idt[i].selector = 0;
        idt[i].zero = 0;
        idt[i].flags = 0;
    }

    /*
     * CPU exception handlers.
     */
    idt_set_gate(0,  (uint32_t)isr0,  0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(1,  (uint32_t)isr1,  0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(2,  (uint32_t)isr2,  0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(3,  (uint32_t)isr3,  0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(4,  (uint32_t)isr4,  0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(5,  (uint32_t)isr5,  0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(6,  (uint32_t)isr6,  0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(7,  (uint32_t)isr7,  0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(8,  (uint32_t)isr8,  0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(9,  (uint32_t)isr9,  0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(10, (uint32_t)isr10, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(11, (uint32_t)isr11, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(12, (uint32_t)isr12, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(13, (uint32_t)isr13, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(14, (uint32_t)isr14, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(15, (uint32_t)isr15, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(16, (uint32_t)isr16, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(17, (uint32_t)isr17, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(18, (uint32_t)isr18, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(19, (uint32_t)isr19, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(20, (uint32_t)isr20, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(21, (uint32_t)isr21, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(22, (uint32_t)isr22, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(23, (uint32_t)isr23, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(24, (uint32_t)isr24, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(25, (uint32_t)isr25, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(26, (uint32_t)isr26, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(27, (uint32_t)isr27, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(28, (uint32_t)isr28, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(29, (uint32_t)isr29, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(30, (uint32_t)isr30, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(31, (uint32_t)isr31, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);


   idt_set_gate(32, (uint32_t)irq0, 0x08,
                IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(33, (uint32_t)irq1, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(34, (uint32_t)irq2, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(35, (uint32_t)irq3, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(36, (uint32_t)irq4, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(37, (uint32_t)irq5, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(38, (uint32_t)irq6, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(39, (uint32_t)irq7, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(40, (uint32_t)irq8, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(41, (uint32_t)irq9, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(42, (uint32_t)irq10, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(43, (uint32_t)irq11, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(44, (uint32_t)irq12, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(45, (uint32_t)irq13, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(46, (uint32_t)irq14, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);

    idt_set_gate(47, (uint32_t)irq15, 0x08,
                 IDT_PRESENT | IDT_RING0 | IDT_INTERRUPT_32);
     /*
     * Load IDTR.
     */
    idt_flush((uint32_t)&idt_ptr);
}
