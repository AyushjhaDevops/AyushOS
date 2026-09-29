#include <stdint.h>

#include <kernel/terminal.h>
#include <kernel/kprintf.h>
#include <kernel/keyboard.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/pic.h>
#include <kernel/irq.h>
#include <kernel/timer.h>

/*
 * Process characters received from the keyboard buffer.
 */
static void process_keyboard_input(void)
{
    while (keyboard_has_char())
    {
        char character = keyboard_getchar();

        /*
         * Backspace handling will be improved later.
         */
        if (character == '\b')
            continue;

        terminal_putchar(character);
    }
}

/*
 * Kernel entry point.
 */
void kernel_main(void)
{
    /*
     * ========================================================
     * TERMINAL
     * ========================================================
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
     * ========================================================
     * GDT
     * ========================================================
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
     * ========================================================
     * IDT
     * ========================================================
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
     * ========================================================
     * PIC
     * ========================================================
     */

    pic_initialize();

    klog_ok(
        "PIC initialized"
    );

    klog_info(
        "PIC remapped to vectors 32-47"
    );

    /*
     * ========================================================
     * IRQ SUBSYSTEM
     * ========================================================
     */

    irq_initialize();

    klog_ok(
        "IRQ subsystem initialized"
    );

    /*
     * ========================================================
     * TIMER
     * ========================================================
     */

    timer_initialize();

    /*
     * ========================================================
     * KEYBOARD
     * ========================================================
     */

    keyboard_initialize();

    klog_ok(
        "PS/2 keyboard initialized"
    );

    klog_info(
        "Keyboard IRQ1 registered"
    );

    /*
     * ========================================================
     * ENABLE HARDWARE INTERRUPTS
     * ========================================================
     */

    klog_info(
        "Enabling hardware interrupts"
    );

    __asm__ volatile ("sti");

    klog_ok(
        "Hardware interrupts enabled"
    );

    /*
     * ========================================================
     * SHELL
     * ========================================================
     */

    terminal_write(
        "\nAyushOS> "
    );

    /*
     * ========================================================
     * PHASE 8 TIMING TEST
     * ========================================================
     *
     * We print a status message once every second.
     *
     * The timer interrupt continues to run in the
     * background.
     */

    uint32_t last_second = 0;

    while (1)
    {
        /*
         * Process keyboard input.
         */
        process_keyboard_input();

        /*
         * Read current uptime.
         */
        uint32_t milliseconds =
            timer_get_milliseconds();

        uint32_t current_second =
            milliseconds / 1000U;

        /*
         * Print once per second.
         */
        if (current_second != last_second)
        {
            last_second = current_second;

            kprintf(
                "\n[TIME] uptime=%u ms ticks=%u\n",
                milliseconds,
                timer_get_ticks()
            );

            terminal_write(
                "AyushOS> "
            );
        }

        /*
         * Wait for an interrupt.
         *
         * Normally this will be the PIT's IRQ0.
         */
        __asm__ volatile ("hlt");
    }
}
