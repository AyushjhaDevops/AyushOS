#ifndef AYUSHOS_PROCESS_H
#define AYUSHOS_PROCESS_H

#include <stdint.h>

#define PROCESS_MAX 16
#define PROCESS_STACK_SIZE 4096

#define PROCESS_INVALID_PID 0

typedef enum
{
    PROCESS_UNUSED = 0,
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_BLOCKED,
    PROCESS_TERMINATED
} process_state_t;

typedef void (*process_entry_t)(void);

/*
 * CPU context saved by context_switch.asm.
 *
 * The order is important.
 *
 * context_switch.asm must save and restore
 * registers in exactly this order.
 */
typedef struct __attribute__((packed))
{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;

    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t eip;
    uint32_t eflags;

    uint32_t cs;
    uint32_t ds;
} process_context_t;

typedef struct
{
    uint32_t pid;

    char name[32];

    process_state_t state;

    process_entry_t entry;

    /*
     * Dedicated kernel stack.
     */
    uint8_t kernel_stack[PROCESS_STACK_SIZE];

    /*
     * Current stack pointer used by
     * the context switcher.
     */
    uint32_t kernel_stack_pointer;

    /*
     * Saved CPU context.
     */
    process_context_t context;

    /*
     * Scheduler/accounting data.
     */
    uint32_t runtime_ticks;
    uint32_t creation_tick;
    uint32_t wakeup_tick;

    uint32_t priority;
    uint32_t scheduling_count;

} process_t;


/*
 * Process subsystem.
 */
void process_initialize(void);

uint32_t process_create(
    const char *name,
    process_entry_t entry,
    uint32_t priority
);


/*
 * Process lookup.
 */
process_t *process_get(uint32_t pid);

process_t *process_current(void);

uint32_t process_current_pid(void);


/*
 * State management.
 */
void process_set_state(
    uint32_t pid,
    process_state_t state
);

void process_terminate(uint32_t pid);


/*
 * Timer integration.
 */
void process_tick(void);


/*
 * Process statistics.
 */
uint32_t process_count(void);

void process_list(void);


/*
 * Context switching.
 */
void process_switch_to(
    uint32_t pid
);

void process_switch_next(void);


/*
 * Process bootstrap.
 *
 * This function is used as the initial
 * execution target for newly created
 * processes.
 */
void process_bootstrap(void);

#endif
