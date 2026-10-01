#include <stdint.h>

#include <kernel/process.h>
#include <kernel/kprintf.h>
#include <kernel/timer.h>


/* ============================================================
 * Process table
 * ============================================================ */

static process_t process_table[PROCESS_MAX];

static uint32_t current_pid =
    PROCESS_INVALID_PID;

static uint32_t next_pid = 1;

static uint32_t active_processes = 0;


/* ============================================================
 * Scheduler configuration
 * ============================================================ */

#define DEFAULT_QUANTUM_TICKS 10

static uint32_t quantum_ticks =
    DEFAULT_QUANTUM_TICKS;

static uint32_t current_slice_ticks = 0;

static uint32_t context_switches = 0;


/* ============================================================
 * Internal helpers
 * ============================================================ */

static uint32_t process_stack_top(
    process_t *process)
{
    return
        (uint32_t)(
            process->kernel_stack +
            PROCESS_STACK_SIZE
        );
}


/*
 * Calculate effective priority.
 *
 * Aging only applies to READY processes.
 *
 * Example:
 *
 * base priority = 2
 * waiting       = 40
 * aging         = 20
 *
 * effective = 2 + 40/20
 *           = 4
 */
static uint32_t calculate_effective_priority(
    process_t *process)
{
    if (process == 0)
    {
        return 0;
    }

    uint32_t effective =
        process->priority;

    if (
        process->state ==
        PROCESS_READY
    )
    {
        effective +=
            process->waiting_ticks /
            PROCESS_AGING_INTERVAL;
    }

    if (
        effective >
        PROCESS_MAX_PRIORITY
    )
    {
        effective =
            PROCESS_MAX_PRIORITY;
    }

    return effective;
}


/*
 * Prepare the initial interrupt-return frame.
 */
static void process_initialize_stack(
    process_t *process)
{
    uint32_t stack_top;

    if (process == 0)
    {
        return;
    }

    stack_top =
        process_stack_top(process);

    stack_top &= ~0x0FU;


    /*
     * EFLAGS
     */
    stack_top -= 4;
    *(uint32_t *)stack_top =
        0x202;


    /*
     * CS
     */
    stack_top -= 4;
    *(uint32_t *)stack_top =
        0x08;


    /*
     * EIP
     */
    stack_top -= 4;
    *(uint32_t *)stack_top =
        (uint32_t)process_bootstrap;


    /*
     * Error code.
     */
    stack_top -= 4;
    *(uint32_t *)stack_top =
        0;


    /*
     * Interrupt number.
     */
    stack_top -= 4;
    *(uint32_t *)stack_top =
        32;


    /*
     * EAX
     */
    stack_top -= 4;
    *(uint32_t *)stack_top =
        0;


    /*
     * ECX
     */
    stack_top -= 4;
    *(uint32_t *)stack_top =
        0;


    /*
     * EDX
     */
    stack_top -= 4;
    *(uint32_t *)stack_top =
        0;


    /*
     * EBX
     */
    stack_top -= 4;
    *(uint32_t *)stack_top =
        0;


    /*
     * Original ESP.
     *
     * POPAD ignores this value.
     */
    stack_top -= 4;
    *(uint32_t *)stack_top =
        0;


    /*
     * EBP
     */
    stack_top -= 4;
    *(uint32_t *)stack_top =
        0;


    /*
     * ESI
     */
    stack_top -= 4;
    *(uint32_t *)stack_top =
        0;


    /*
     * EDI
     */
    stack_top -= 4;
    *(uint32_t *)stack_top =
        0;


    /*
     * DS
     */
    stack_top -= 4;
    *(uint32_t *)stack_top =
        0x10;


    process->kernel_stack_pointer =
        stack_top;
}


/* ============================================================
 * Process bootstrap
 * ============================================================ */

void process_bootstrap(void)
{
    process_t *process =
        process_current();

    if (process != 0)
    {
        kprintf(
            "\n[PROCESS] Starting PID=%u (%s) "
            "BASE=%u EFFECTIVE=%u\n",
            process->pid,
            process->name,
            process->priority,
            process->effective_priority
        );


        /*
         * First-run response metric.
         */
        if (
            process->first_run_tick == 0
        )
        {
            process->first_run_tick =
                timer_get_ticks();

            process->response_ticks =
                process->first_run_tick -
                process->creation_tick;
        }


        if (process->entry != 0)
        {
            process->entry();
        }


        /*
         * If process entry returns,
         * terminate the process.
         */
        process_terminate(
            process->pid
        );
    }


    /*
     * Wait for the scheduler.
     */
    while (1)
    {
        __asm__ volatile ("hlt");
    }
}


/* ============================================================
 * Initialization
 * ============================================================ */

void process_initialize(void)
{
    for (
        uint32_t i = 0;
        i < PROCESS_MAX;
        i++
    )
    {
        process_table[i].pid = 0;

        process_table[i].state =
            PROCESS_UNUSED;
    }


    current_pid =
        PROCESS_INVALID_PID;

    next_pid = 1;

    active_processes = 0;

    current_slice_ticks = 0;

    context_switches = 0;

    quantum_ticks =
        DEFAULT_QUANTUM_TICKS;


    klog_ok(
        "Process subsystem initialized"
    );


    kprintf(
        "[SCHED] Algorithm: Priority + Aging\n"
    );

    kprintf(
        "[SCHED] Quantum: %u ticks\n",
        quantum_ticks
    );

    kprintf(
        "[SCHED] Aging interval: %u ticks\n",
        PROCESS_AGING_INTERVAL
    );
}


/* ============================================================
 * Process creation
 * ============================================================ */

uint32_t process_create(
    const char *name,
    process_entry_t entry,
    uint32_t priority)
{
    if (entry == 0)
    {
        return PROCESS_INVALID_PID;
    }


    if (
        priority >
        PROCESS_MAX_PRIORITY
    )
    {
        priority =
            PROCESS_MAX_PRIORITY;
    }


    uint32_t slot =
        PROCESS_MAX;


    for (
        uint32_t i = 0;
        i < PROCESS_MAX;
        i++
    )
    {
        if (
            process_table[i].state ==
            PROCESS_UNUSED
        )
        {
            slot = i;
            break;
        }
    }


    if (slot == PROCESS_MAX)
    {
        klog_error(
            "Process table is full"
        );

        return PROCESS_INVALID_PID;
    }


    process_t *process =
        &process_table[slot];


    /*
     * Clear PCB.
     */
    for (
        uint32_t i = 0;
        i < sizeof(process_t);
        i++
    )
    {
        ((uint8_t *)process)[i] =
            0;
    }


    /*
     * PID.
     */
    process->pid =
        next_pid++;


    if (next_pid == 0)
    {
        next_pid = 1;
    }


    /*
     * Name.
     */
    uint32_t i = 0;

    if (name != 0)
    {
        while (
            name[i] != '\0' &&
            i < sizeof(process->name) - 1
        )
        {
            process->name[i] =
                name[i];

            i++;
        }
    }

    process->name[i] =
        '\0';


    process->entry =
        entry;


    process->priority =
        priority;


    process->effective_priority =
        priority;


    process->state =
        PROCESS_READY;


    process->creation_tick =
        timer_get_ticks();


    process->runtime_ticks = 0;

    process->waiting_ticks = 0;

    process->scheduling_count = 0;

    process->first_run_tick = 0;

    process->response_ticks = 0;

    process->completion_tick = 0;

    process->turnaround_ticks = 0;


    process_initialize_stack(
        process
    );


    active_processes++;


    kprintf(
        "[PROCESS] Created PID=%u "
        "name=%s "
        "priority=%u\n",
        process->pid,
        process->name,
        process->priority
    );


    return process->pid;
}


/* ============================================================
 * Process lookup
 * ============================================================ */

process_t *process_get(
    uint32_t pid)
{
    if (
        pid ==
        PROCESS_INVALID_PID
    )
    {
        return 0;
    }


    for (
        uint32_t i = 0;
        i < PROCESS_MAX;
        i++
    )
    {
        if (
            process_table[i].state !=
            PROCESS_UNUSED &&
            process_table[i].pid ==
            pid
        )
        {
            return
                &process_table[i];
        }
    }


    return 0;
}


/* ============================================================
 * Current process
 * ============================================================ */

process_t *process_current(void)
{
    return process_get(
        current_pid
    );
}


uint32_t process_current_pid(void)
{
    return current_pid;
}


/* ============================================================
 * State
 * ============================================================ */

void process_set_state(
    uint32_t pid,
    process_state_t state)
{
    process_t *process =
        process_get(pid);

    if (process == 0)
    {
        return;
    }


    process->state =
        state;


    if (
        state ==
        PROCESS_RUNNING
    )
    {
        current_pid =
            process->pid;

        current_slice_ticks =
            0;
    }
}


/* ============================================================
 * Termination
 * ============================================================ */

void process_terminate(
    uint32_t pid)
{
    process_t *process =
        process_get(pid);

    if (process == 0)
    {
        return;
    }


    if (
        process->state ==
        PROCESS_TERMINATED
    )
    {
        return;
    }


    process->state =
        PROCESS_TERMINATED;


    process->completion_tick =
        timer_get_ticks();


    process->turnaround_ticks =
        process->completion_tick -
        process->creation_tick;


    if (active_processes > 0)
    {
        active_processes--;
    }


    kprintf(
        "\n[PROCESS] PID=%u terminated\n",
        pid
    );
}


/* ============================================================
 * Runtime accounting
 * ============================================================ */

void process_tick(void)
{
    process_t *current =
        process_current();

    if (current == 0)
    {
        return;
    }


    if (
        current->state ==
        PROCESS_RUNNING
    )
    {
        current->runtime_ticks++;
    }
}


/* ============================================================
 * Find next process
 * ============================================================ */

/*
 * Priority scheduling with aging.
 *
 * Selection rules:
 *
 * 1. Ignore UNUSED/BLOCKED/TERMINATED.
 * 2. Calculate effective priority.
 * 3. Select highest effective priority.
 * 4. If tied, use round-robin order.
 */
static process_t *
process_find_next_ready(void)
{
    process_t *best = 0;

    uint32_t best_priority = 0;


    /*
     * Find current PCB slot.
     */
    uint32_t current_slot = 0;


    for (
        uint32_t i = 0;
        i < PROCESS_MAX;
        i++
    )
    {
        if (
            process_table[i].pid ==
            current_pid
        )
        {
            current_slot = i;
            break;
        }
    }


    /*
     * Scan in circular order.
     *
     * This gives round-robin behavior when
     * effective priorities are equal.
     */
    for (
        uint32_t offset = 1;
        offset <= PROCESS_MAX;
        offset++
    )
    {
        uint32_t index =
            (current_slot + offset) %
            PROCESS_MAX;


        process_t *candidate =
            &process_table[index];


        if (
            candidate->state !=
            PROCESS_READY
        )
        {
            continue;
        }


        candidate->effective_priority =
            calculate_effective_priority(
                candidate
            );


        if (
            best == 0 ||
            candidate->effective_priority >
            best_priority
        )
        {
            best = candidate;

            best_priority =
                candidate->effective_priority;
        }
    }


    return best;
}


/* ============================================================
 * Phase 12 scheduler
 * ============================================================ */

irq_frame_t *
process_schedule_tick(
    irq_frame_t *frame)
{
    if (frame == 0)
    {
        return frame;
    }


    /*
     * Account CPU runtime.
     */
    process_tick();


    process_t *current =
        process_current();


    /*
     * Save current interrupt frame.
     */
    if (current != 0)
    {
        current->kernel_stack_pointer =
            (uint32_t)frame;
    }


    /*
     * Increase waiting time.
     */
    for (
        uint32_t i = 0;
        i < PROCESS_MAX;
        i++
    )
    {
        process_t *process =
            &process_table[i];


        if (
            process->state ==
            PROCESS_READY
        )
        {
            process->waiting_ticks++;


            /*
             * Update effective priority.
             */
            process->effective_priority =
                calculate_effective_priority(
                    process
                );
        }
    }


    current_slice_ticks++;


    /*
     * Current process still has CPU time.
     */
    if (
        current != 0 &&
        current->state ==
        PROCESS_RUNNING &&
        current_slice_ticks <
        quantum_ticks
    )
    {
        return frame;
    }


    /*
     * Select next process.
     */
    process_t *next =
        process_find_next_ready();


    /*
     * Nobody else is READY.
     */
    if (next == 0)
    {
        current_slice_ticks = 0;

        return frame;
    }


    /*
     * Same process.
     */
    if (
        current != 0 &&
        next->pid ==
        current->pid
    )
    {
        current_slice_ticks = 0;

        return frame;
    }


    /*
     * Current process becomes READY.
     */
    if (
        current != 0 &&
        current->state ==
        PROCESS_RUNNING
    )
    {
        current->state =
            PROCESS_READY;
    }


    /*
     * Activate selected process.
     */
    next->state =
        PROCESS_RUNNING;


    current_pid =
        next->pid;


    /*
     * First-run response metric.
     */
    if (
        next->first_run_tick == 0
    )
    {
        next->first_run_tick =
            timer_get_ticks();

        next->response_ticks =
            next->first_run_tick -
            next->creation_tick;
    }


    /*
     * Reset aging after receiving CPU time.
     */
    next->waiting_ticks = 0;

    next->effective_priority =
        next->priority;


    /*
     * Scheduler metrics.
     */
    next->scheduling_count++;

    context_switches++;

    current_slice_ticks = 0;


    /*
     * Debug information.
     */
    if (current != 0)
    {
        kprintf(
            "\n[SCHED] PID %u -> PID %u "
            "BASE=%u EFFECTIVE=%u\n",
            current->pid,
            next->pid,
            next->priority,
            next->effective_priority
        );
    }
    else
    {
        kprintf(
            "\n[SCHED] BOOT -> PID %u "
            "BASE=%u EFFECTIVE=%u\n",
            next->pid,
            next->priority,
            next->effective_priority
        );
    }


    return
        (irq_frame_t *)
        next->kernel_stack_pointer;
}


/* ============================================================
 * Priority API
 * ============================================================ */

uint32_t process_get_effective_priority(
    uint32_t pid)
{
    process_t *process =
        process_get(pid);

    if (process == 0)
    {
        return 0;
    }


    process->effective_priority =
        calculate_effective_priority(
            process
        );


    return
        process->effective_priority;
}


void process_set_priority(
    uint32_t pid,
    uint32_t priority)
{
    process_t *process =
        process_get(pid);

    if (process == 0)
    {
        return;
    }


    if (
        priority >
        PROCESS_MAX_PRIORITY
    )
    {
        priority =
            PROCESS_MAX_PRIORITY;
    }


    process->priority =
        priority;


    process->effective_priority =
        calculate_effective_priority(
            process
        );


    kprintf(
        "[SCHED] PID=%u priority=%u\n",
        pid,
        priority
    );
}


void process_reset_aging(
    uint32_t pid)
{
    process_t *process =
        process_get(pid);

    if (process == 0)
    {
        return;
    }


    process->waiting_ticks = 0;

    process->effective_priority =
        process->priority;
}


/* ============================================================
 * Scheduler configuration
 * ============================================================ */

uint32_t process_get_context_switches(void)
{
    return context_switches;
}


uint32_t process_get_quantum_ticks(void)
{
    return quantum_ticks;
}


void process_set_quantum_ticks(
    uint32_t quantum)
{
    if (quantum == 0)
    {
        return;
    }


    quantum_ticks =
        quantum;


    current_slice_ticks =
        0;
}


/* ============================================================
 * Process count
 * ============================================================ */

uint32_t process_count(void)
{
    return active_processes;
}


/* ============================================================
 * State name
 * ============================================================ */

static const char *
process_state_name(
    process_state_t state)
{
    switch (state)
    {
        case PROCESS_UNUSED:
            return "UNUSED";

        case PROCESS_READY:
            return "READY";

        case PROCESS_RUNNING:
            return "RUNNING";

        case PROCESS_BLOCKED:
            return "BLOCKED";

        case PROCESS_TERMINATED:
            return "TERMINATED";

        default:
            return "UNKNOWN";
    }
}


/* ============================================================
 * Process table
 * ============================================================ */

void process_list(void)
{
    kprintf(
        "\nPID   STATE       BASE  EFF   "
        "RUNTIME   WAIT   SCHED   NAME\n"
    );


    kprintf(
        "------------------------------------------------------------\n"
    );


    for (
        uint32_t i = 0;
        i < PROCESS_MAX;
        i++
    )
    {
        process_t *process =
            &process_table[i];


        if (
            process->state ==
            PROCESS_UNUSED
        )
        {
            continue;
        }


        kprintf(
            "%u     %-10s  %u     %u     "
            "%u        %u      %u      %s\n",
            process->pid,
            process_state_name(
                process->state
            ),
            process->priority,
            process->effective_priority,
            process->runtime_ticks,
            process->waiting_ticks,
            process->scheduling_count,
            process->name
        );
    }


    kprintf(
        "\nContext switches: %u\n",
        context_switches
    );


    kprintf(
        "Quantum: %u ticks\n",
        quantum_ticks
    );
}


/* ============================================================
 * Compatibility functions
 * ============================================================ */

void process_switch_to(
    uint32_t pid)
{
    process_t *target =
        process_get(pid);

    if (target == 0)
    {
        return;
    }


    if (
        target->state !=
        PROCESS_READY
    )
    {
        return;
    }


    process_t *current =
        process_current();


    if (current != 0)
    {
        if (
            current->state ==
            PROCESS_RUNNING
        )
        {
            current->state =
                PROCESS_READY;
        }
    }


    target->state =
        PROCESS_RUNNING;


    current_pid =
        target->pid;


    target->scheduling_count++;

    current_slice_ticks =
        0;


    context_switches++;
}


void process_switch_next(void)
{
    process_t *next =
        process_find_next_ready();

    if (next != 0)
    {
        process_switch_to(
            next->pid
        );
    }
}
