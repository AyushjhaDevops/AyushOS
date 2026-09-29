#include <stdint.h>

#include <kernel/process.h>
#include <kernel/timer.h>
#include <kernel/kprintf.h>


static process_t process_table[PROCESS_MAX];

static uint32_t current_pid =
    PROCESS_INVALID_PID;

static uint32_t next_pid = 1;

static uint32_t active_process_count = 0;


/*
 * Assembly context switcher.
 *
 * old_esp:
 *      Location where current process ESP
 *      will be saved.
 *
 * new_esp:
 *      ESP of the next process.
 */
extern void context_switch(
    uint32_t *old_esp,
    uint32_t new_esp
);


/*
 * Copy process name.
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
 * Find unused PCB.
 */
static process_t *process_find_free_slot(void)
{
    uint32_t i;

    for (i = 0; i < PROCESS_MAX; i++)
    {
        if (
            process_table[i].state ==
            PROCESS_UNUSED
        )
        {
            return &process_table[i];
        }
    }

    return 0;
}


/*
 * Return top of kernel stack.
 */
static uint32_t process_stack_top(
    process_t *process)
{
    if (process == 0)
        return 0;

    return (uint32_t)(
        &process->kernel_stack[
            PROCESS_STACK_SIZE
        ]
    );
}


/*
 * Process bootstrap.
 *
 * Every newly created process starts here.
 *
 * It calls the process entry function.
 *
 * When the entry function returns,
 * the process is terminated.
 */
void process_bootstrap(void)
{
    process_t *process =
        process_current();

    if (process == 0)
    {
        while (1)
        {
            __asm__ volatile ("hlt");
        }
    }

    /*
     * Execute the actual process.
     */
    if (process->entry != 0)
    {
        process->entry();
    }

    /*
     * If the process function returns,
     * terminate it.
     */
    process_terminate(
        process->pid
    );

    /*
     * There is currently no full scheduler
     * in Phase 10.
     *
     * Stay halted until Phase 11.
     */
    while (1)
    {
        __asm__ volatile ("hlt");
    }
}


/*
 * Prepare initial stack.
 *
 * The stack is constructed so that
 * context_switch() can eventually execute
 * a normal RET into process_bootstrap().
 */
static void process_initialize_stack(
    process_t *process)
{
    uint32_t stack_top;

    if (process == 0)
        return;

    /*
     * Start at the top of the process's
     * dedicated kernel stack.
     */
    stack_top =
        process_stack_top(process);

    /*
     * Align stack to 16 bytes.
     */
    stack_top &= ~0x0FU;


    /*
     * ----------------------------------------
     * Construct the stack expected by:
     *
     *     popfd
     *     popad
     *     ret
     *
     * ----------------------------------------
     *
     * Final memory layout:
     *
     * ESP + 0   EFLAGS
     * ESP + 4   EDI
     * ESP + 8   ESI
     * ESP + 12  EBP
     * ESP + 16  ESP
     * ESP + 20  EBX
     * ESP + 24  EDX
     * ESP + 28  ECX
     * ESP + 32  EAX
     * ESP + 36  return address
     *
     * Therefore construct it backwards.
     */


    /*
     * Return address.
     */
    stack_top -= 4;

    *(uint32_t *)stack_top =
        (uint32_t)process_bootstrap;


    /*
     * EAX.
     */
    stack_top -= 4;

    *(uint32_t *)stack_top = 0;


    /*
     * ECX.
     */
    stack_top -= 4;

    *(uint32_t *)stack_top = 0;


    /*
     * EDX.
     */
    stack_top -= 4;

    *(uint32_t *)stack_top = 0;


    /*
     * EBX.
     */
    stack_top -= 4;

    *(uint32_t *)stack_top = 0;


    /*
     * Original ESP value.
     *
     * This value is restored into the
     * CPU's ESP register by POPAD.
     *
     * It is ignored by POPAD itself.
     */
    stack_top -= 4;

    *(uint32_t *)stack_top = 0;


    /*
     * EBP.
     */
    stack_top -= 4;

    *(uint32_t *)stack_top = 0;


    /*
     * ESI.
     */
    stack_top -= 4;

    *(uint32_t *)stack_top = 0;


    /*
     * EDI.
     */
    stack_top -= 4;

    *(uint32_t *)stack_top = 0;


    /*
     * EFLAGS.
     *
     * 0x202:
     *
     * bit 1 = reserved and normally 1
     * bit 9 = IF (interrupt enable)
     */
    stack_top -= 4;

    *(uint32_t *)stack_top =
        0x202;


    /*
     * Save the prepared stack pointer.
     */
    process->kernel_stack_pointer =
        stack_top;


    /*
     * Metadata used for debugging and
     * future scheduler work.
     */
    process->context.eip =
        (uint32_t)process_bootstrap;

    process->context.eflags =
        0x202;

    process->context.cs =
        0x08;

    process->context.ds =
        0x10;
}
/*
 * Create process.
 */
uint32_t process_create(
    const char *name,
    process_entry_t entry,
    uint32_t priority)
{
    process_t *process;

    if (entry == 0)
        return PROCESS_INVALID_PID;

    process =
        process_find_free_slot();

    if (process == 0)
    {
        klog_error(
            "Process table is full"
        );

        return PROCESS_INVALID_PID;
    }

    /*
     * Clear PCB.
     */
    uint8_t *bytes =
        (uint8_t *)process;

    for (
        uint32_t i = 0;
        i < sizeof(process_t);
        i++
    )
    {
        bytes[i] = 0;
    }

    process->pid =
        next_pid++;

    if (next_pid ==
        PROCESS_INVALID_PID)
    {
        next_pid = 1;
    }

    process_copy_name(
        process->name,
        name
    );

    process->entry =
        entry;

    process->state =
        PROCESS_READY;

    process->priority =
        priority;

    process->runtime_ticks = 0;

    process->creation_tick =
        timer_get_ticks();

    process->wakeup_tick = 0;

    process->scheduling_count = 0;

    /*
     * Prepare initial CPU stack.
     */
    process_initialize_stack(
        process
    );

    active_process_count++;

    return process->pid;
}


/*
 * Initialize process manager.
 */
void process_initialize(void)
{
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

    current_pid =
        PROCESS_INVALID_PID;

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

    klog_ok(
        "Process context initialization enabled"
    );
}


/*
 * Find process by PID.
 */
process_t *process_get(
    uint32_t pid)
{
    if (pid ==
        PROCESS_INVALID_PID)
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
 * Current process.
 */
process_t *process_current(void)
{
    return process_get(
        current_pid
    );
}


/*
 * Current PID.
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

    process->state =
        state;

    if (
        state ==
        PROCESS_RUNNING
    )
    {
        current_pid =
            pid;
    }
}


/*
 * Terminate process.
 */
void process_terminate(
    uint32_t pid)
{
    process_t *process =
        process_get(pid);

    if (process == 0)
        return;

    if (
        process->state ==
        PROCESS_TERMINATED
    )
    {
        return;
    }

    process->state =
        PROCESS_TERMINATED;

    if (active_process_count > 0)
    {
        active_process_count--;
    }

    if (current_pid == pid)
    {
        current_pid =
            PROCESS_INVALID_PID;
    }
}


/*
 * Timer accounting.
 */
void process_tick(void)
{
    process_t *process =
        process_current();

    if (process == 0)
        return;

    if (
        process->state ==
        PROCESS_RUNNING
    )
    {
        process->runtime_ticks++;
    }
}


/*
 * Switch directly to a process.
 *
 * This is the first real context switch
 * primitive in AyushOS.
 */
void process_switch_to(
    uint32_t pid)
{
    process_t *old_process;
    process_t *new_process;

    if (pid ==
        PROCESS_INVALID_PID)
    {
        return;
    }

    new_process =
        process_get(pid);

    if (new_process == 0)
    {
        return;
    }

    if (
        new_process->state ==
        PROCESS_TERMINATED
    )
    {
        return;
    }

    old_process =
        process_current();

    /*
     * If there is no current process,
     * simply establish the first process.
     */
    if (old_process == 0)
    {
        current_pid =
            pid;

        new_process->state =
            PROCESS_RUNNING;

        new_process->scheduling_count++;

        return;
    }

    /*
     * Same process.
     */
    if (
        old_process->pid ==
        new_process->pid
    )
    {
        return;
    }

    /*
     * Current process becomes READY.
     */
    if (
        old_process->state ==
        PROCESS_RUNNING
    )
    {
        old_process->state =
            PROCESS_READY;
    }

    /*
     * New process becomes RUNNING.
     */
    new_process->state =
        PROCESS_RUNNING;

    current_pid =
        new_process->pid;

    new_process->scheduling_count++;

    /*
     * Perform actual CPU context switch.
     */
    context_switch(
        &old_process->kernel_stack_pointer,
        new_process->kernel_stack_pointer
    );
}


/*
 * Find next READY process.
 */
static process_t *process_find_next_ready(void)
{
    process_t *current =
        process_current();

    uint32_t start = 0;

    if (current != 0)
    {
        start =
            current->pid % PROCESS_MAX;
    }

    /*
     * First pass.
     */
    for (
        uint32_t offset = 1;
        offset <= PROCESS_MAX;
        offset++
    )
    {
        uint32_t index =
            (start + offset) %
            PROCESS_MAX;

        process_t *candidate =
            &process_table[index];

        if (
            candidate->state ==
            PROCESS_READY
        )
        {
            return candidate;
        }
    }

    return 0;
}


/*
 * Switch to next READY process.
 */
void process_switch_next(void)
{
    process_t *next =
        process_find_next_ready();

    if (next == 0)
        return;

    process_switch_to(
        next->pid
    );
}


/*
 * Number of active processes.
 */
uint32_t process_count(void)
{
    return active_process_count;
}


/*
 * State name.
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
 * Display process table.
 */
void process_list(void)
{
    kprintf(
        "\nPID   STATE       PRIORITY   RUNTIME   SCHEDULES   NAME\n"
    );

    kprintf(
        "--------------------------------------------------------\n"
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
            "%u     %s     %u          %u        %u          %s\n",
            process->pid,
            process_state_name(
                process->state
            ),
            process->priority,
            process->runtime_ticks,
            process->scheduling_count,
            process->name
        );
    }

    kprintf(
        "--------------------------------------------------------\n"
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
