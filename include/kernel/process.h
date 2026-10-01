#ifndef AYUSHOS_PROCESS_H
#define AYUSHOS_PROCESS_H

#include <stdint.h>

#include <kernel/irq.h>

#define PROCESS_MAX 16
#define PROCESS_STACK_SIZE 4096

#define PROCESS_INVALID_PID 0

/*
 * Priority range.
 *
 * Higher number = higher base priority.
 */
#define PROCESS_MIN_PRIORITY 0
#define PROCESS_MAX_PRIORITY 10

/*
 * Every N waiting ticks, a READY process receives
 * one additional effective-priority point.
 */
#define PROCESS_AGING_INTERVAL 20

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
 * Process Control Block.
 */
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
     * Pointer to the saved interrupt frame.
     */
    uint32_t kernel_stack_pointer;


    /*
     * Scheduling properties.
     */
    uint32_t priority;

    uint32_t effective_priority;


    /*
     * Runtime statistics.
     */
    uint32_t runtime_ticks;

    uint32_t waiting_ticks;

    uint32_t creation_tick;

    uint32_t wakeup_tick;


    /*
     * Scheduling statistics.
     */
    uint32_t scheduling_count;


    /*
     * Response-time metrics.
     */
    uint32_t first_run_tick;

    uint32_t response_ticks;


    /*
     * Completion metrics.
     */
    uint32_t completion_tick;

    uint32_t turnaround_ticks;

} process_t;


/* ============================================================
 * Process subsystem
 * ============================================================ */

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


/* ============================================================
 * Phase 11/12 scheduler
 * ============================================================ */

irq_frame_t *process_schedule_tick(
    irq_frame_t *frame
);

uint32_t process_get_context_switches(void);

uint32_t process_get_quantum_ticks(void);

void process_set_quantum_ticks(
    uint32_t quantum
);


/* ============================================================
 * Phase 12 priority API
 * ============================================================ */

uint32_t process_get_effective_priority(
    uint32_t pid
);

void process_set_priority(
    uint32_t pid,
    uint32_t priority
);

void process_reset_aging(
    uint32_t pid
);


/* ============================================================
 * Compatibility
 * ============================================================ */

void process_switch_to(uint32_t pid);

void process_switch_next(void);

void process_bootstrap(void);

#endif
