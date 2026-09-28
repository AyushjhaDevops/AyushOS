#include <stdint.h>
#include <kernel/terminal.h>
#include <kernel/kprintf.h>

void kernel_main(void)
{
    terminal_initialize();

    klog_ok("Terminal initialized");
    klog_ok("VGA text mode ready");

    klog_info("Kernel entry point reached");
    klog_info("Phase 2: Terminal subsystem");

    kprintf("\n");
    kprintf("Kernel information:\n");
    kprintf("  VGA address : %x\n", 0xB8000);
    kprintf("  Screen      : %ux%u\n",
            VGA_WIDTH,
            VGA_HEIGHT);

    kprintf("\n");

    klog_ok("Kernel initialization complete");

    terminal_write("\nAyushOS> ");

    while (1)
    {
        __asm__ volatile ("hlt");
    }
}
