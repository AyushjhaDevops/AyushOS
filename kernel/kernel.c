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
 * Process A.
 */
static void test_process_a(void)
{
    while (1)
    {
        kprintf(
            "\n[PROC-A] PID=%u executing\n",
            process_current_pid()
        );

        /*
         * Halt until an interrupt arrives.
         */
        __asm__ volatile ("hlt");
    }
}


/*
 * Process B.
 */
static void test_process_b(void)
{
    while (1)
    {
        kprintf(
            "\n[PROC-B] PID=%u executing\n",
            process_current_pid()
        );

        __asm__ volatile ("hlt");
    }
}


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
 * Keyboard processing.
 */
static void process_keyboard_input(void)
{
    while (keyboard_has_char())
    {
        char character =
            keyboard_getchar();

        if (character == '\b')
            continue;

        terminal_putchar(
            character
        );
    }
}


/*
 * Kernel entry.
 */
void kernel_main(void)
{
    /*
     * -------------------------
     * Terminal
     * -------------------------
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
     * -------------------------
     * GDT
     * -------------------------
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
     * -------------------------
     * IDT
     * -------------------------
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
     * -------------------------
     * PIC
     * -------------------------
     */
    pic_initialize();

    klog_ok(
        "PIC initialized"
    );

    klog_info(
        "PIC remapped to vectors 32-47"
    );


    /*
     * -------------------------
     * IRQ subsystem
     * -------------------------
     */
    irq_initialize();

    klog_ok(
        "IRQ subsystem initialized"
    );


    /*
     * -------------------------
     * Process subsystem
     * -------------------------
     */
    process_initialize();


    /*
     * -------------------------
     * Timer
     * -------------------------
     */
    timer_initialize();


    /*
     * -------------------------
     * Keyboard
     * -------------------------
     */
    keyboard_initialize();

    klog_ok(
        "PS/2 keyboard initialized"
    );

    klog_info(
        "Keyboard IRQ1 registered"
    );


    /*
     * -------------------------
     * Create processes
     * -------------------------
     */

    uint32_t idle_pid =
        process_create(
            "idle",
            idle_process,
            0
        );

    uint32_t process_a_pid =
        process_create(
            "test-A",
            test_process_a,
            10
        );

    uint32_t process_b_pid =
        process_create(
            "test-B",
            test_process_b,
            10
        );


    /*
     * Creation results.
     */
    if (idle_pid == 0)
    {
        klog_error(
            "Failed to create idle"
        );
    }
    else
    {
        kprintf(
            "[ OK ] Created idle PID=%u\n",
            idle_pid
        );
    }


    if (process_a_pid == 0)
    {
        klog_error(
            "Failed to create test-A"
        );
    }
    else
    {
        kprintf(
            "[ OK ] Created test-A PID=%u\n",
            process_a_pid
        );
    }


    if (process_b_pid == 0)
    {
        klog_error(
            "Failed to create test-B"
        );
    }
    else
    {
        kprintf(
            "[ OK ] Created test-B PID=%u\n",
            process_b_pid
        );
    }


    /*
     * Establish first running process.
     *
     * We intentionally select test-A.
     */
    process_set_state(
        process_a_pid,
        PROCESS_RUNNING
    );


    kprintf(
        "\n[PROCESS] Initial process selected: PID=%u\n",
        process_current_pid()
    );


    /*
     * Display process table.
     */
    process_list();


    /*
     * Enable interrupts.
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
     * ------------------------------------------------
     * PHASE 10 MANUAL CONTEXT SWITCH DEMONSTRATION
     * ------------------------------------------------
     *
     * We allow the kernel to run for approximately
     * one second and then manually switch to B.
     *
     * Phase 11 will automate this through the scheduler.
     */

    uint32_t switch_stage = 0;

    uint32_t last_second = 0;


    while (1)
    {
        process_keyboard_input();


        uint32_t milliseconds =
            timer_get_milliseconds();


        uint32_t current_second =
            milliseconds / 1000U;


        /*
         * After two seconds:
         *
         * A -> B
         */
        if (
            current_second >= 2
            &&
            switch_stage == 0
        )
        {
            switch_stage = 1;

            kprintf(
                "\n[CONTEXT] Switching from PID=%u to PID=%u\n",
                process_current_pid(),
                process_b_pid
            );

            process_switch_to(
                process_b_pid
            );

            /*
             * Execution may never return here
             * until process B performs another
             * context switch.
             */
        }


        /*
         * Kernel status once per second.
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

            uint32_t runtime = 0;

            if (current != 0)
            {
                runtime =
                    current->runtime_ticks;
            }

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
        }


        /*
         * Wait for the next interrupt.
         */
        __asm__ volatile ("hlt");
    }
}
