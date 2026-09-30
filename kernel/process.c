#include <stdint.h>

#include <kernel/process.h>
#include <kernel/kprintf.h>
#include <kernel/timer.h>


/* ============================================================
 * Process table
 * ============================================================ */

static process_t process_table[PROCESS_MAX];

static uint32_t current_pid = PROCESS_INVALID_PID;

static uint32_t next_pid = 1;

static uint32_t active_processes = 0;


/* ============================================================
 * Scheduler state
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
 * Create an initial interrupt frame.
 *
 * irq_common expects the stack to look like:
 *
 * ESP -> DS
 *        EDI
 *        ESI
 *        EBP
 *        saved ESP
 *        EBX
 *        EDX
 *        ECX
 *        EAX
 *        INT_NO
 *        ERR_CODE
 *        EIP
 *        CS
 *        EFLAGS
 *
 * The IRQ return path will therefore start the process
 * through IRETD.
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

    /*
     * Keep stack reasonably aligned.
     */
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
     * Error code
     */
    stack_top -= 4;

    *(uint32_t *)stack_top =
        0;


    /*
     * Interrupt number.
     *
     * The exact value is not important for IRETD,
     * but keeping a valid-looking timer vector makes
     * the frame easy to inspect in GDB.
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
     * Original ESP field from PUSHAD.
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


    /*
     * This is the exact ESP value that irq_common
     * expects when restoring the process.
     */
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
            "\n[PROCESS] Starting PID=%u (%s)\n",
            process->pid,
            process->name
        );

        /*
         * First-run response metric.
         */
        if (process->first_run_tick == 0)
        {
            process->first_run_tick =
                timer_get_ticks();

            process->response_ticks =
                process->first_run_tick -
                process->creation_tick;
        }

        /*
         * Execute the actual process entry.
         */
        if (process->entry != 0)
        {
            process->entry();
        }

        /*
         * If the entry function ever returns,
         * terminate the process.
         */
        process_terminate(
            process->pid
        );
    }

    /*
     * A terminated process waits for the next
     * scheduler interrupt to move elsewhere.
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
    for (uint32_t i = 0; i < PROCESS_MAX; i++)
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
        "[SCHED] Round-robin quantum: %u ticks\n",
        quantum_ticks
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

    uint32_t slot = PROCESS_MAX;

    for (uint32_t i = 0; i < PROCESS_MAX; i++)
    {
        if (process_table[i].state ==
            PROCESS_UNUSED)
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
        ((uint8_t *)process)[i] = 0;
    }


    /*
     * Assign PID.
     */
    process->pid = next_pid++;

    if (next_pid == 0)
    {
        next_pid = 1;
    }


    /*
     * Copy process name.
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

    process->name[i] = '\0';


    process->entry =
        entry;

    process->priority =
        priority;

    process->state =
        PROCESS_READY;

    process->creation_tick =
        timer_get_ticks();

    process->wakeup_tick =
        0;

    process->runtime_ticks =
        0;

    process->waiting_ticks =
        0;

    process->scheduling_count =
        0;

    process->first_run_tick =
        0;

    process->response_ticks =
        0;

    process->completion_tick =
        0;

    process->turnaround_ticks =
        0;


    /*
     * Prepare initial IRQ-resume stack.
     */
    process_initialize_stack(
        process
    );


    active_processes++;


    kprintf(
        "[PROCESS] Created PID=%u name=%s priority=%u\n",
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
    if (pid == PROCESS_INVALID_PID)
    {
        return 0;
    }

    for (uint32_t i = 0; i < PROCESS_MAX; i++)
    {
        if (
            process_table[i].state !=
            PROCESS_UNUSED &&
            process_table[i].pid == pid
        )
        {
            return &process_table[i];
        }
    }

    return 0;
}


/* ============================================================
 * Current process
 * ============================================================ */

process_t *process_current(void)
{
    return process_get(current_pid);
}


uint32_t process_current_pid(void)
{
    return current_pid;
}


/* ============================================================
 * State management
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

    if (state == PROCESS_RUNNING)
    {
        current_pid =
            process->pid;
    }
}


/* ============================================================
 * Process termination
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

    /*
     * IMPORTANT:
     *
     * Do not clear current_pid if this is the
     * currently executing process.
     *
     * The timer scheduler needs to see that the
     * current process became TERMINATED and select
     * another process.
     */
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
 * Scheduler helper
 * ============================================================ */

static process_t *process_find_next_ready(void)
{
    if (current_pid == 0)
    {
        /*
         * No current process.
         */
        for (uint32_t i = 0;
             i < PROCESS_MAX;
             i++)
        {
            if (
                process_table[i].state ==
                PROCESS_READY
            )
            {
                return &process_table[i];
            }
        }

        return 0;
    }


    /*
     * Find current slot.
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
     * Round-robin search.
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

        if (
            process_table[index].state ==
            PROCESS_READY
        )
        {
            return &process_table[index];
        }
    }

    return 0;
}


/* ============================================================
 * Phase 11 scheduler
 * ============================================================ */

irq_frame_t *process_schedule_tick(
    irq_frame_t *frame)
{
    if (frame == 0)
    {
        return frame;
    }


    /*
     * Account for the current process.
     */
    process_tick();


    process_t *current =
        process_current();


    /*
     * Save the current interrupt frame.
     *
     * This is the complete CPU state needed to resume
     * the current process later.
     */
    if (current != 0)
    {
        current->kernel_stack_pointer =
            (uint32_t)frame;
    }


    /*
     * Account waiting time for READY processes.
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
        }
    }


    /*
     * Increase current time slice.
     */
    current_slice_ticks++;


    /*
     * Do not switch until the quantum expires,
     * unless the current process is no longer runnable.
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
     * Find next READY process.
     */
    process_t *next =
        process_find_next_ready();


    /*
     * Nothing else is runnable.
     */
    if (next == 0)
    {
        current_slice_ticks = 0;

        return frame;
    }


    /*
     * If the only runnable process is the current
     * one, keep executing it.
     */
    if (
        current != 0 &&
        next->pid == current->pid
    )
    {
        current_slice_ticks = 0;

        return frame;
    }


    /*
     * Current process becomes READY if it was still
     * running.
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
     * Activate next process.
     */
    next->state =
        PROCESS_RUNNING;

    current_pid =
        next->pid;


    /*
     * First-run response metric.
     */
    if (next->first_run_tick == 0)
    {
        next->first_run_tick =
            timer_get_ticks();

        next->response_ticks =
            next->first_run_tick -
            next->creation_tick;
    }


    /*
     * Scheduler statistics.
     */
    next->scheduling_count++;

    context_switches++;

    current_slice_ticks = 0;


    /*
     * Debug message.
     *
     * Do not print on every timer tick.
     * This only happens on an actual context switch.
     */
    if (current != 0)
    {
        kprintf(
            "\n[SCHED] PID %u -> PID %u\n",
            current->pid,
            next->pid
        );
    }
    else
    {
        kprintf(
            "\n[SCHED] BOOT -> PID %u\n",
            next->pid
        );
    }


    /*
     * Return the new process's saved IRQ frame.
     *
     * irq_common.asm will load ESP with this value
     * and restore the selected process.
     */
    return (irq_frame_t *)
        next->kernel_stack_pointer;
}


/* ============================================================
 * Scheduler statistics
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

    current_slice_ticks = 0;
}


/* ============================================================
 * Process count
 * ============================================================ */

uint32_t process_count(void)
{
    return active_processes;
}


/* ============================================================
 * Process table display
 * ============================================================ */

static const char *process_state_name(
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


void process_list(void)
{
    kprintf(
        "\nPID   STATE       PRI   RUNTIME   WAIT   SCHED   NAME\n"
    );

    kprintf(
        "----------------------------------------------------------\n"
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
            "%u     %-10s  %u     %u        %u      %u      %s\n",
            process->pid,
            process_state_name(
                process->state
            ),
            process->priority,
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
 * Legacy Phase 10 compatibility
 * ============================================================ */

/*
 * Phase 11 no longer uses the old cooperative context_switch()
 * for timer-driven scheduling.
 *
 * Keep these functions only so existing code does not break.
 *
 * The scheduler should use process_schedule_tick().
 */

void process_switch_to(uint32_t pid)
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

    context_switches++;

    current_slice_ticks = 0;
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
