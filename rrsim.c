#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

#define MAX_PROG_LEN 10 // Max terms in a "program"
#define MAX_PROCS 10 // Max number of processes
#define QUANTUM 40 // Time quantum, ms

#define MIN(x,y) ((x)<(y)?(x):(y)) // Compute the minimum

enum process_state {
    READY,
    SLEEPING,
    EXITED
};

struct process {
    int pid;
    enum process_state state;
    int sleep_time_remaining;
    int awake_time_remaining;
    int pc;
    int program[MAX_PROG_LEN + 1];
};

static struct process table[MAX_PROCS];

/**
 * Count how many processes have not exited yet.
 */
int count_active_processes(int proc_count)
{
    int i;
    int active = 0;

    for (i = 0; i < proc_count; i++) {
        if (table[i].state != EXITED)
            active++;
    }

    return active;
}

/**
 * Subtract elapsed time from all sleeping processes.
 */
void update_sleeping_processes(int proc_count, int elapsed)
{
    int i;

    for (i = 0; i < proc_count; i++) {
        if (table[i].state == SLEEPING)
            table[i].sleep_time_remaining -= elapsed;
    }
}

/**
 * Find the smallest remaining sleep time among sleeping processes.
 */
int next_wakeup_time(int proc_count)
{
    int i;
    int found = 0;
    int min_sleep = 0;

    for (i = 0; i < proc_count; i++) {
        if (table[i].state == SLEEPING) {
            if (!found || table[i].sleep_time_remaining < min_sleep) {
                min_sleep = table[i].sleep_time_remaining;
                found = 1;
            }
        }
    }

    return min_sleep;
}

/**
 * Wake up any sleeping processes whose timers have expired.
 */
void wake_ready_processes(struct queue *q, int proc_count)
{
    int i;

    for (i = 0; i < proc_count; i++) {
        if (table[i].state == SLEEPING && table[i].sleep_time_remaining <= 0) {
            table[i].pc++;

            if (table[i].program[table[i].pc] == 0) {
                table[i].state = EXITED;
                printf("PID %d: Exiting\n", table[i].pid);
            }
            else {
                table[i].state = READY;
                table[i].awake_time_remaining = table[i].program[table[i].pc];
                printf("PID %d: Waking up for %d ms\n",
                       table[i].pid, table[i].awake_time_remaining);
                queue_enqueue(q, &table[i]);
            }
        }
    }
}

/**
 * Parse one command-line program into a process table entry.
 */
void parse_program(char *arg, int pid)
{
    char *token;
    int index = 0;

    table[pid].pid = pid;
    table[pid].state = READY;
    table[pid].sleep_time_remaining = 0;
    table[pid].awake_time_remaining = 0;
    table[pid].pc = 0;

    token = strtok(arg, ",");

    while (token != NULL && index < MAX_PROG_LEN) {
        table[pid].program[index] = atoi(token);
        token = strtok(NULL, ",");
        index++;
    }

    table[pid].program[index] = 0;
    table[pid].awake_time_remaining = table[pid].program[0];
}

/**
 * Run one process for one time slice or until it blocks/exits.
 */
int run_process(struct queue *q, struct process *p, int proc_count)
{
    int run_time = MIN(QUANTUM, p->awake_time_remaining);

    p->awake_time_remaining -= run_time;
    update_sleeping_processes(proc_count, run_time);

    if (p->awake_time_remaining > 0) {
        queue_enqueue(q, p);
    }
    else {
        p->pc++;

        if (p->program[p->pc] == 0) {
            p->state = EXITED;
            printf("PID %d: Exiting\n", p->pid);
        }
        else {
            p->state = SLEEPING;
            p->sleep_time_remaining = p->program[p->pc];
            printf("PID %d: Sleeping for %d ms\n", p->pid, p->sleep_time_remaining);
        }
    }

    return run_time;
}

/**
 * Main.
 */
int main(int argc, char **argv)
{
    int clock = 0;
    int proc_count = argc - 1;
    int i;

    struct queue *q = queue_new();

    for (i = 0; i < proc_count; i++) {
        parse_program(argv[i + 1], i);
        queue_enqueue(q, &table[i]);
    }

    while (count_active_processes(proc_count) > 0) {
        struct process *runner;
        int elapsed;

        if (queue_is_empty(q)) {
            elapsed = next_wakeup_time(proc_count);
            clock += elapsed;
            update_sleeping_processes(proc_count, elapsed);

            printf("=== Clock %d ms ===\n", clock);
            wake_ready_processes(q, proc_count);
        }

        runner = queue_dequeue(q);

        if (runner == NULL)
            continue;

        printf("=== Clock %d ms ===\n", clock);
        wake_ready_processes(q, proc_count);

        printf("PID %d: Running\n", runner->pid);
        elapsed = run_process(q, runner, proc_count);
        printf("PID %d: Ran for %d ms\n", runner->pid, elapsed);

        clock += elapsed;
    }

    queue_free(q);
}