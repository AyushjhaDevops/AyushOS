#include <stdint.h>

#include <kernel/terminal.h>
#include <kernel/kprintf.h>
#include <kernel/keyboard.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/pic.h>
#include <kernel/irq.h>
#include <kernel/timer.h>
#include <kernel/process.h>

/*
 * ============================================================
 * TEST PROCESSES
 * ============================================================
 *
 * These functions are NOT executed in Phase 9.
 *
 * They are process entry points stored inside the PCB.
 *
 * Phase 10 will introduce context switching and actually
 * transfer CPU execution to these functions.
 */

/*
 * Idle process.
 */
static void idle_process(void)
{
    while (1)
    {
        __asm__ volatile ("hlt");
    }
}

/*
 * Test process A.
 */
static void test_process_a(void)
{
    while (1)
    {
        /*
         * Phase 10 will eventually allow this process
         * to execute independently.
         */
        __asm__ volatile ("hlt");
    }
}

/*
 * Test process B.
 */
static void test_process_b(void)
{
    while (1)
    {
        __asm__ volatile ("hlt");
    }
}

/*
 * Process keyboard characters.
 */
static void process_keyboard_input(void)
{
    while (keyboard_has_char())
    {
        char character =
            keyboard_getchar();

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
     * IRQ
     * ========================================================
     */

    irq_initialize();

    klog_ok(
        "IRQ subsystem initialized"
    );

    /*
     * ========================================================
     * PROCESS MANAGER
     * ========================================================
     *
     * Initialize before enabling hardware interrupts.
     */

    process_initialize();

    /*
     * ========================================================
     * PIT
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
     * CREATE PROCESSES
     * ========================================================
     */

    uint32_t idle_pid =
        process_create(
            "idle",
            idle_process,
            0
        );

    uint32_t test_a_pid =
        process_create(
            "test-A",
            test_process_a,
            10
        );

    uint32_t test_b_pid =
        process_create(
            "test-B",
            test_process_b,
            10
        );

    if (idle_pid == 0)
    {
        klog_error(
            "Failed to create idle process"
        );
    }
    else
    {
        kprintf(
            "[ OK ] Created idle process PID=%u\n",
            idle_pid
        );
    }

    if (test_a_pid == 0)
    {
        klog_error(
            "Failed to create test-A"
        );
    }
    else
    {
        kprintf(
            "[ OK ] Created test-A PID=%u\n",
            test_a_pid
        );
    }

    if (test_b_pid == 0)
    {
        klog_error(
            "Failed to create test-B"
        );
    }
    else
    {
        kprintf(
            "[ OK ] Created test-B PID=%u\n",
            test_b_pid
        );
    }

    /*
     * ========================================================
     * SELECT INITIAL PROCESS
     * ========================================================
     *
     * Phase 9 does not switch CPU execution.
     *
     * We only mark PID 1 as RUNNING so IRQ0 can account
     * runtime ticks against it.
     */

    process_set_state(
        idle_pid,
        PROCESS_RUNNING
    );

    /*
     * Other processes remain READY.
     */

    /*
     * ========================================================
     * ENABLE INTERRUPTS
     * ========================================================
     */

    klog_info(
        "Enabling hardware interrupts"
    );

    __asm__ volatile ("sti");

    klog_ok(
        "Hardware interrupts enabled"
    );

    terminal_write(
        "\nAyushOS> "
    );

    /*
     * ========================================================
     * PHASE 9 DEMONSTRATION
     * ========================================================
     */

    uint32_t last_second = 0;

    uint32_t process_table_printed = 0;

    while (1)
    {
        /*
         * Keyboard input.
         */
        process_keyboard_input();

        /*
         * Current system time.
         */
        uint32_t milliseconds =
            timer_get_milliseconds();

        uint32_t current_second =
            milliseconds / 1000U;

        /*
         * Print process table once after
         * the first second.
         */
        if (
            current_second >= 1
            &&
            process_table_printed == 0
        )
        {
            process_table_printed = 1;

            kprintf(
                "\n[PROCESS] Process subsystem online\n"
            );

            process_list();

            terminal_write(
                "\nAyushOS> "
            );
        }

        /*
         * Print timer/process accounting once per second.
         */
        if (
            current_second !=
            last_second
        )
        {
            last_second =
                current_second;

            process_t *current =
                process_current();

            uint32_t runtime =
                0;

            if (current != 0)
                runtime =
                    current->runtime_ticks;

            kprintf(
                "\n[TIME] uptime=%u ms ticks=%u\n",
                milliseconds,
                timer_get_ticks()
            );

            kprintf(
                "[PROCESS] current_pid=%u runtime_ticks=%u\n",
                process_current_pid(),
                runtime
            );

            terminal_write(
                "AyushOS> "
            );
        }

        /*
         * Wait for an interrupt.
         *
         * PIT IRQ0 occurs approximately every 10 ms.
         */
        __asm__ volatile ("hlt");
    }
}
