#ifndef AYUSHOS_PROCESS_H
#define AYUSHOS_PROCESS_H

#include <stdint.h>

#include <kernel/irq.h>

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
     * Points to an irq_frame_t stored inside kernel_stack.
     *
     * When a process is preempted, this points to the exact
     * interrupt frame needed to resume it.
     */
    uint32_t kernel_stack_pointer;

    /*
     * Scheduler statistics.
     */
    uint32_t runtime_ticks;
    uint32_t creation_tick;
    uint32_t wakeup_tick;

    uint32_t priority;

    uint32_t scheduling_count;

    uint32_t waiting_ticks;

    uint32_t first_run_tick;
    uint32_t response_ticks;

    uint32_t completion_tick;
    uint32_t turnaround_ticks;

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

process_t *process_get(uint32_t pid);

process_t *process_current(void);

uint32_t process_current_pid(void);

void process_set_state(
    uint32_t pid,
    process_state_t state
);

void process_terminate(uint32_t pid);

void process_tick(void);

uint32_t process_count(void);

void process_list(void);


/*
 * Phase 11 scheduler interface.
 */
irq_frame_t *process_schedule_tick(
    irq_frame_t *frame
);

uint32_t process_get_context_switches(void);

uint32_t process_get_quantum_ticks(void);

void process_set_quantum_ticks(
    uint32_t quantum
);


/*
 * Kept for compatibility with Phase 10.
 *
 * Phase 11 uses interrupt-frame scheduling instead.
 */
void process_switch_to(uint32_t pid);

void process_switch_next(void);

void process_bootstrap(void);

#endif
