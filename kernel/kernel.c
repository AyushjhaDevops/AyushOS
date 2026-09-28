#include <stdint.h>

#include <kernel/terminal.h>
#include <kernel/kprintf.h>
#include <kernel/keyboard.h>

static void process_keyboard_input(void)
{
    while (keyboard_has_char())
    {
        char character = keyboard_getchar();

        /*
         * Basic terminal backspace handling.
         */
        if (character == '\b')
        {
            /*
             * For Phase 3, simply move the cursor
             * one position backward and erase the
             * character.
             *
             * A proper terminal editing layer will
             * come later.
             */
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
    klog_info("Phase 3: PS/2 keyboard driver");

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
