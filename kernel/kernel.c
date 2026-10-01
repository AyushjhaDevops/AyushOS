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
 * Low priority process
 * ============================================================ */

static void test_process_low(void)
{
    uint32_t counter = 0;

    while (1)
    {
        counter++;

        if (
            (counter % 1000000U) == 0
        )
        {
            kprintf(
                "\n[LOW] PID=%u counter=%u\n",
                process_current_pid(),
                counter
            );
        }
    }
}


/* ============================================================
 * Medium priority process
 * ============================================================ */

static void test_process_medium(void)
{
    uint32_t counter = 0;

    while (1)
    {
        counter++;

        if (
            (counter % 1000000U) == 0
        )
        {
            kprintf(
                "\n[MEDIUM] PID=%u counter=%u\n",
                process_current_pid(),
                counter
            );
        }
    }
}


/* ============================================================
 * High priority process
 * ============================================================ */

static void test_process_high(void)
{
    uint32_t counter = 0;

    while (1)
    {
        counter++;

        if (
            (counter % 1000000U) == 0
        )
        {
            kprintf(
                "\n[HIGH] PID=%u counter=%u\n",
                process_current_pid(),
                counter
            );
        }
    }
}


/* ============================================================
 * Idle process
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
        "        AyushOS - Phase 12\n"
        " Priority Scheduling + Aging\n"
        "========================================\n\n"
    );


    /*
     * CPU structures.
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
     * IRQ.
     */
    irq_initialize();


    /*
     * Keyboard.
     */
    keyboard_initialize();


    /*
     * Process subsystem.
     */
    process_initialize();


    /*
     * PID 1:
     *
     * Kernel/idle context.
     */
    uint32_t idle_pid =
        process_create(
            "idle",
            idle_process,
            0
        );


    process_set_state(
        idle_pid,
        PROCESS_RUNNING
    );


    /*
     * PID 2:
     *
     * Lowest priority worker.
     */
    uint32_t low_pid =
        process_create(
            "low",
            test_process_low,
            1
        );


    /*
     * PID 3:
     *
     * Medium priority worker.
     */
    uint32_t medium_pid =
        process_create(
            "medium",
            test_process_medium,
            5
        );


    /*
     * PID 4:
     *
     * Highest priority worker.
     */
    uint32_t high_pid =
        process_create(
            "high",
            test_process_high,
            8
        );


    kprintf(
        "\n[BOOT] Process configuration:\n"
    );


    kprintf(
        "       PID=%u idle     priority=0\n",
        idle_pid
    );


    kprintf(
        "       PID=%u low      priority=1\n",
        low_pid
    );


    kprintf(
        "       PID=%u medium   priority=5\n",
        medium_pid
    );


    kprintf(
        "       PID=%u high     priority=8\n",
        high_pid
    );


    /*
     * Initial table.
     */
    process_list();


    /*
     * Start PIT.
     */
    timer_initialize();


    kprintf(
        "\n[SCHED] Priority scheduling enabled\n"
    );


    kprintf(
        "[SCHED] Aging enabled\n"
    );


    kprintf(
        "[SCHED] Aging interval = %u ticks\n",
        PROCESS_AGING_INTERVAL
    );


    kprintf(
        "[SCHED] Quantum = %u ticks\n",
        process_get_quantum_ticks()
    );


    /*
     * Enable hardware interrupts.
     */
    __asm__ volatile ("sti");


    kprintf(
        "\n[BOOT] Interrupts enabled\n"
    );


    kprintf(
        "[BOOT] Phase 12 scheduler running...\n\n"
    );


    /*
     * Kernel/idle context.
     */
    while (1)
    {
        __asm__ volatile ("hlt");
    }
}
