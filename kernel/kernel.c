#include <stdint.h>

#include <kernel/terminal.h>
#include <kernel/kprintf.h>
#include <kernel/keyboard.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/pic.h>
#include <kernel/irq.h>

static void process_keyboard_input(void)
{
    while (keyboard_has_char())
    {
        char character =
            keyboard_getchar();

        /*
         * Handle backspace.
         *
         * We will improve terminal editing
         * in a later phase.
         */
        if (character == '\b')
        {
            continue;
        }

        terminal_putchar(character);
    }
}

void kernel_main(void)
{
    /*
     * --------------------------------------------------
     * Terminal
     * --------------------------------------------------
     */

    terminal_initialize();

    klog_ok(
        "Terminal initialized"
    );

    klog_ok(
        "VGA text mode ready"
    );

    klog_info(
        "Kernel entry point reached"
    );

    /*
     * --------------------------------------------------
     * GDT
     * --------------------------------------------------
     */

    gdt_initialize();

    klog_ok(
        "GDT initialized"
    );

    klog_info(
        "Kernel code segment: 0x08"
    );

    klog_info(
        "Kernel data segment: 0x10"
    );

    /*
     * --------------------------------------------------
     * IDT
     * --------------------------------------------------
     */

    idt_initialize();

    klog_ok(
        "IDT initialized"
    );

    klog_info(
        "CPU exception handlers installed"
    );

    klog_info(
        "Hardware IRQ vectors 32-47 installed"
    );

    /*
     * --------------------------------------------------
     * PIC
     * --------------------------------------------------
     */

    pic_initialize();

    klog_ok(
        "PIC initialized"
    );

    klog_info(
        "PIC remapped to vectors 32-47"
    );

    /*
     * --------------------------------------------------
     * IRQ subsystem
     * --------------------------------------------------
     */

    irq_initialize();

    klog_ok(
        "IRQ subsystem initialized"
    );

    /*
     * --------------------------------------------------
     * Keyboard
     * --------------------------------------------------
     */

    keyboard_initialize();

    klog_ok(
        "PS/2 keyboard initialized"
    );

    klog_info(
        "Keyboard IRQ1 registered"
    );

    /*
     * --------------------------------------------------
     * Enable CPU hardware interrupts
     * --------------------------------------------------
     */

    klog_info(
        "Enabling hardware interrupts"
    );

    __asm__ volatile (
        "sti"
    );

    klog_ok(
        "Hardware interrupts enabled"
    );

    terminal_write(
        "\nAyushOS> "
    );

    /*
     * --------------------------------------------------
     * Kernel main loop
     * --------------------------------------------------
     */

    while (1)
    {
        /*
         * Wait for the next interrupt.
         *
         * Keyboard IRQ1 will wake the CPU.
         */
        __asm__ volatile (
            "hlt"
        );

        /*
         * Process characters that were placed
         * into the keyboard buffer by IRQ1.
         */
        process_keyboard_input();
    }
}
