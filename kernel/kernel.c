#include <stdint.h>

#include <kernel/terminal.h>
#include <kernel/kprintf.h>
#include <kernel/keyboard.h>
#include <kernel/gdt.h>

static void process_keyboard_input(void)
{
    while (keyboard_has_char())
    {
        char character = keyboard_getchar();

        /*
         * Backspace handling will be implemented
         * properly in the terminal/shell layer later.
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
    terminal_initialize();

    klog_ok("Terminal initialized");
    klog_ok("VGA text mode ready");

    klog_info("Kernel entry point reached");
    klog_info("Phase 4: Global Descriptor Table");

    /*
     * Initialize our own GDT before continuing
     * with the rest of kernel initialization.
     */
    gdt_initialize();

    klog_ok("GDT initialized");
    klog_info("Kernel code segment: 0x08");
    klog_info("Kernel data segment: 0x10");

    keyboard_initialize();

    klog_ok("PS/2 keyboard initialized");
    klog_info("Scancode Set 1 decoder ready");
    klog_info("Keyboard buffer ready");

    terminal_write("\nAyushOS> ");

    while (1)
    {
        keyboard_poll();
        process_keyboard_input();
    }
}
