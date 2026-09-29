#ifndef AYUSHOS_PROCESS_H
#define AYUSHOS_PROCESS_H

#include <stdint.h>

/*
 * Maximum number of processes supported by the
 * Phase 9 static process table.
 */
#define PROCESS_MAX 16

/*
 * Kernel stack size for every process.
 *
 * 4096 bytes = 4 KiB.
 */
#define PROCESS_STACK_SIZE 4096

/*
 * Special PID representing an invalid process.
 */
#define PROCESS_INVALID_PID 0

/*
 * Process states.
 */
typedef enum
{
    PROCESS_UNUSED = 0,
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_BLOCKED,
    PROCESS_TERMINATED
} process_state_t;

/*
 * Function executed when a process eventually runs.
 */
typedef void (*process_entry_t)(void);

/*
 * Saved CPU context.
 *
 * Phase 9 only defines the structure.
 *
 * Phase 10 will use this structure during actual
 * context switching.
 */
typedef struct __attribute__((packed))
{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t eip;
    uint32_t eflags;

    uint32_t cs;
    uint32_t ds;
} process_context_t;

/*
 * Process Control Block.
 */
typedef struct
{
    /*
     * Process identification.
     */
    uint32_t pid;

    /*
     * Human-readable process name.
     */
    char name[32];

    /*
     * Current process state.
     */
    process_state_t state;

    /*
     * Entry point of the process.
     */
    process_entry_t entry;

    /*
     * Kernel stack.
     */
    uint8_t kernel_stack[PROCESS_STACK_SIZE];

    /*
     * Current kernel stack pointer.
     *
     * Phase 10 will actively use this.
     */
    uint32_t kernel_stack_pointer;

    /*
     * Saved CPU context.
     *
     * Phase 10 will populate/use this during
     * context switching.
     */
    process_context_t context;

    /*
     * Number of timer ticks during which the
     * process has executed.
     */
    uint32_t runtime_ticks;

    /*
     * Tick at which process was created.
     */
    uint32_t creation_tick;

    /*
     * Tick at which the process should wake.
     *
     * Phase 9 does not yet perform automatic
     * sleeping/wakeup scheduling.
     */
    uint32_t wakeup_tick;

    /*
     * Scheduling priority.
     *
     * Phase 11 will use this.
     */
    uint32_t priority;

    /*
     * Number of times this process has been selected
     * by the scheduler.
     *
     * Phase 11 will use this.
     */
    uint32_t scheduling_count;
} process_t;

/*
 * Initialize process management.
 */
void process_initialize(void);

/*
 * Create a kernel process.
 *
 * Returns:
 *
 *     PID > 0 on success
 *     0 on failure
 */
uint32_t process_create(
    const char *name,
    process_entry_t entry,
    uint32_t priority
);

/*
 * Find a process by PID.
 */
process_t *process_get(uint32_t pid);

/*
 * Return currently selected process.
 */
process_t *process_current(void);

/*
 * Return current PID.
 */
uint32_t process_current_pid(void);

/*
 * Change process state.
 */
void process_set_state(
    uint32_t pid,
    process_state_t state
);

/*
 * Terminate a process.
 */
void process_terminate(uint32_t pid);

/*
 * Called from the timer interrupt on every tick.
 *
 * This performs accounting only.
 *
 * It does NOT switch processes yet.
 */
void process_tick(void);

/*
 * Return number of active process slots.
 */
uint32_t process_count(void);

/*
 * Print process table.
 */
void process_list(void);

#endif
