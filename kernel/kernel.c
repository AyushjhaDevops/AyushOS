#include <stdint.h>

#include <kernel/kprintf.h>
#include <kernel/terminal.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/pic.h>
#include <kernel/irq.h>
#include <kernel/timer.h>
#include <kernel/keyboard.h>
#include <kernel/process.h>


/* ============================================================
 * Test Process A
 * ============================================================ */

static void test_process_a(void)
{
    uint32_t counter = 0;

    while (1)
    {
        counter++;

        /*
         * Print occasionally so the terminal does not
         * get flooded.
         */
        if (
            (counter % 1000000U) ==
            0
        )
        {
            kprintf(
                "\n[PROC-A] PID=%u counter=%u\n",
                process_current_pid(),
                counter
            );
        }
    }
}


/* ============================================================
 * Test Process B
 * ============================================================ */

static void test_process_b(void)
{
    uint32_t counter = 0;

    while (1)
    {
        counter++;

        if (
            (counter % 1000000U) ==
            0
        )
        {
            kprintf(
                "\n[PROC-B] PID=%u counter=%u\n",
                process_current_pid(),
                counter
            );
        }
    }
}


/* ============================================================
 * Idle / boot context
 * ============================================================ */

static void idle_process(void)
{
    while (1)
    {
        __asm__ volatile ("hlt");
    }
}


/* ============================================================
 * Kernel main
 * ============================================================ */

void kernel_main(void)
{
    terminal_initialize();

    kprintf(
        "\n"
        "========================================\n"
        "        AyushOS - Phase 11\n"
        "  Preemptive Round-Robin Scheduler\n"
        "========================================\n\n"
    );


    /*
     * CPU descriptor tables.
     */
    gdt_initialize();

    klog_ok(
        "GDT initialized"
    );


    idt_initialize();

    klog_ok(
        "IDT initialized"
    );


    /*
     * PIC.
     */
    pic_initialize();

    klog_ok(
        "PIC initialized"
    );


    /*
     * IRQ subsystem.
     */
    irq_initialize();


    /*
     * Keyboard.
     */
    keyboard_initialize();


    /*
     * Process subsystem MUST be initialized before
     * timer scheduling starts.
     */
    process_initialize();


    /*
     * Create boot/idle context.
     *
     * PID 1 represents the kernel_main execution context.
     *
     * Its actual entry function is not used for the initial
     * execution because kernel_main is already running.
     */
    uint32_t idle_pid =
        process_create(
            "idle",
            idle_process,
            0
        );


    /*
     * The kernel itself is currently executing.
     *
     * Treat it as PID 1.
     */
    process_set_state(
        idle_pid,
        PROCESS_RUNNING
    );


    /*
     * Test processes.
     */
    uint32_t test_a_pid =
        process_create(
            "test-A",
            test_process_a,
            1
        );


    uint32_t test_b_pid =
        process_create(
            "test-B",
            test_process_b,
            1
        );


    kprintf(
        "\n[BOOT] Created processes:\n"
    );

    kprintf(
        "       PID=%u idle\n",
        idle_pid
    );

    kprintf(
        "       PID=%u test-A\n",
        test_a_pid
    );

    kprintf(
        "       PID=%u test-B\n",
        test_b_pid
    );


    /*
     * Display initial process table.
     */
    process_list();


    /*
     * PIT timer.
     *
     * This registers IRQ0 and enables timer interrupts.
     */
    timer_initialize();


    kprintf(
        "\n[SCHED] Preemptive scheduling enabled\n"
    );

    kprintf(
        "[SCHED] Time quantum = %u ticks\n",
        process_get_quantum_ticks()
    );

    kprintf(
        "[SCHED] Timer = %u Hz\n",
        100U
    );


    /*
     * Enable CPU interrupts.
     */
    __asm__ volatile ("sti");


    kprintf(
        "\n[BOOT] Interrupts enabled\n"
    );

    kprintf(
        "[BOOT] Scheduler is now running...\n\n"
    );


    /*
     * Kernel main becomes the idle/boot context.
     *
     * Timer IRQs will save this execution context into PID 1
     * and switch to PID 2 / PID 3.
     */
    while (1)
    {
        /*
         * Sleep until the next interrupt.
         */
        __asm__ volatile ("hlt");
    }
}
