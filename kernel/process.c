#include <stdint.h>

#include <kernel/process.h>
#include <kernel/timer.h>
#include <kernel/kprintf.h>

/*
 * Static process table.
 *
 * Dynamic memory allocation is intentionally not used yet.
 *
 * Later phases can replace this with kmalloc/slab-style
 * allocation once the memory subsystem is more mature.
 */
static process_t process_table[PROCESS_MAX];

/*
 * Currently selected process.
 *
 * Phase 9 starts with PID 1 as the kernel idle process.
 */
static uint32_t current_pid = PROCESS_INVALID_PID;

/*
 * Next PID to allocate.
 */
static uint32_t next_pid = 1;

/*
 * Number of allocated process slots.
 */
static uint32_t active_process_count = 0;

/*
 * Copy a string into a fixed-size process name buffer.
 *
 * We don't use libc's strcpy because AyushOS is freestanding.
 */
static void process_copy_name(
    char *destination,
    const char *source)
{
    uint32_t i;

    if (destination == 0)
        return;

    if (source == 0)
    {
        destination[0] = '\0';
        return;
    }

    for (i = 0; i < 31; i++)
    {
        if (source[i] == '\0')
            break;

        destination[i] = source[i];
    }

    destination[i] = '\0';
}

/*
 * Find an unused slot.
 */
static process_t *process_find_free_slot(void)
{
    uint32_t i;

    for (i = 0; i < PROCESS_MAX; i++)
    {
        if (process_table[i].state == PROCESS_UNUSED)
            return &process_table[i];
    }

    return 0;
}

/*
 * Calculate the initial top of the kernel stack.
 */
static uint32_t process_stack_top(
    process_t *process)
{
    if (process == 0)
        return 0;

    return (
        uint32_t)(
            &process->kernel_stack[
                PROCESS_STACK_SIZE
            ]
        );
}

/*
 * Initialize the process context.
 *
 * This is the context that Phase 10's context switcher
 * will eventually load.
 */
static void process_initialize_context(
    process_t *process)
{
    if (process == 0)
        return;

    /*
     * Start with a clean context.
     */
    process->context.edi = 0;
    process->context.esi = 0;
    process->context.ebp = 0;

    /*
     * Stack starts at the top of the process's
     * dedicated kernel stack.
     */
    process->context.esp =
        process_stack_top(process);

    process->context.ebx = 0;
    process->context.edx = 0;
    process->context.ecx = 0;
    process->context.eax = 0;

    /*
     * Entry point.
     *
     * Phase 10 will arrange the stack/return path so
     * that the process begins executing here.
     */
    process->context.eip =
        (uint32_t)process->entry;

    /*
     * Interrupt flag enabled.
     *
     * 0x202:
     *
     * bit 1  = reserved
     * bit 9  = IF
     */
    process->context.eflags = 0x202;

    /*
     * Kernel code segment.
     */
    process->context.cs = 0x08;

    /*
     * Kernel data segment.
     */
    process->context.ds = 0x10;
}

/*
 * Create a new kernel process.
 */
uint32_t process_create(
    const char *name,
    process_entry_t entry,
    uint32_t priority)
{
    process_t *process;

    /*
     * Entry function is mandatory.
     */
    if (entry == 0)
        return PROCESS_INVALID_PID;

    /*
     * Find free process slot.
     */
    process = process_find_free_slot();

    if (process == 0)
    {
        klog_error(
            "Process table is full"
        );

        return PROCESS_INVALID_PID;
    }

    /*
     * Clear the complete PCB.
     */
    uint8_t *bytes = (uint8_t *)process;

    for (
        uint32_t i = 0;
        i < sizeof(process_t);
        i++
    )
    {
        bytes[i] = 0;
    }

    /*
     * Allocate PID.
     */
    process->pid = next_pid++;

    /*
     * Protect against PID wrapping to zero.
     */
    if (next_pid == PROCESS_INVALID_PID)
        next_pid = 1;

    /*
     * Set process metadata.
     */
    process_copy_name(
        process->name,
        name
    );

    process->entry = entry;

    process->state = PROCESS_READY;

    process->priority = priority;

    process->runtime_ticks = 0;

    process->creation_tick =
        timer_get_ticks();

    process->wakeup_tick = 0;

    process->scheduling_count = 0;

    /*
     * Prepare initial CPU context.
     */
    process_initialize_context(
        process
    );

    active_process_count++;

    return process->pid;
}

/*
 * Initialize process management.
 */
void process_initialize(void)
{
    /*
     * Clear process table.
     */
    uint8_t *bytes =
        (uint8_t *)process_table;

    for (
        uint32_t i = 0;
        i < sizeof(process_table);
        i++
    )
    {
        bytes[i] = 0;
    }

    /*
     * Reset subsystem state.
     */
    current_pid = PROCESS_INVALID_PID;

    next_pid = 1;

    active_process_count = 0;

    klog_ok(
        "Process manager initialized"
    );

    klog_info(
        "Process table capacity: 16"
    );

    klog_info(
        "Kernel stack size: 4096 bytes"
    );
}

/*
 * Find process by PID.
 */
process_t *process_get(uint32_t pid)
{
    if (pid == PROCESS_INVALID_PID)
        return 0;

    for (
        uint32_t i = 0;
        i < PROCESS_MAX;
        i++
    )
    {
        if (
            process_table[i].state !=
                PROCESS_UNUSED
            &&
            process_table[i].pid == pid
        )
        {
            return &process_table[i];
        }
    }

    return 0;
}

/*
 * Return current process.
 */
process_t *process_current(void)
{
    return process_get(
        current_pid
    );
}

/*
 * Return current PID.
 */
uint32_t process_current_pid(void)
{
    return current_pid;
}

/*
 * Change process state.
 */
void process_set_state(
    uint32_t pid,
    process_state_t state)
{
    process_t *process =
        process_get(pid);

    if (process == 0)
        return;

    process->state = state;

    /*
     * If this process is marked running,
     * remember it as the current process.
     *
     * Actual CPU execution switching is NOT performed.
     */
    if (state == PROCESS_RUNNING)
    {
        current_pid = pid;
    }
}

/*
 * Terminate process.
 */
void process_terminate(uint32_t pid)
{
    process_t *process =
        process_get(pid);

    if (process == 0)
        return;

    /*
     * Don't immediately reuse the PCB.
     *
     * Keep it as TERMINATED so the process table
     * remains inspectable.
     */
    process->state =
        PROCESS_TERMINATED;

    if (active_process_count > 0)
        active_process_count--;

    /*
     * If the current process was terminated,
     * clear current PID.
     *
     * A future scheduler will select another process.
     */
    if (current_pid == pid)
        current_pid =
            PROCESS_INVALID_PID;
}

/*
 * Process accounting.
 *
 * Called from IRQ0.
 */
void process_tick(void)
{
    process_t *process =
        process_current();

    if (process == 0)
        return;

    /*
     * Only a running process receives CPU
     * runtime accounting.
     */
    if (
        process->state ==
        PROCESS_RUNNING
    )
    {
        process->runtime_ticks++;
    }
}

/*
 * Return active process count.
 */
uint32_t process_count(void)
{
    return active_process_count;
}

/*
 * Convert state enum to printable text.
 */
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

/*
 * Print process table.
 */
void process_list(void)
{
    kprintf(
        "\nPID   STATE       PRIORITY   RUNTIME   NAME\n"
    );

    kprintf(
        "---------------------------------------------\n"
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
            "%u     %s     %u          %u       %s\n",
            process->pid,
            process_state_name(
                process->state
            ),
            process->priority,
            process->runtime_ticks,
            process->name
        );
    }

    kprintf(
        "---------------------------------------------\n"
    );

    kprintf(
        "Active processes: %u\n",
        process_count()
    );

    kprintf(
        "Current PID: %u\n",
        process_current_pid()
    );
}
