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

    (void) clock;

    queue_free(q);
}