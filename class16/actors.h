#ifndef ACT_PRINT_H
#define ACT_PRINT_H
#include "fifo.h"
#include <assert.h>
#include <stdio.h>

void actor_mul(fifo_t *F1, fifo_t *F2, fifo_t *q);
void actor_print(fifo_t *F);

// our stuff
void actor_inc(fifo_t *F1, fifo_t *q);
void actor_fork(fifo_t *F1, fifo_t *q1, fifo_t *q2);

#endif
