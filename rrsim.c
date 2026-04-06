#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

#define MAX_PROG_LEN 10 // Max terms in a "program"
#define MAX_PROCS 10 // Max number of processes
#define QUANTUM 40 // Time quantum, ms

#define MIN(x,y) ((x)<(y)?(x):(y)) // Compute the minimum

// TODO: all the code!

/**
 * Main.
 */
int main(int argc, char **argv)
{
    int clock = 0;

    struct queue *q = queue_new();

    // TODO
    queue_free(q);
}
