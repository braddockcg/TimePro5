#ifndef TIMEPRO_PRIVATE_H
#define TIMEPRO_PRIVATE_H

#include "platform.h"

#define STACK_SIZE 1000
#define NAME_SIZE 150
#define HEAP_SIZE 200000
#define MAX_NUM_THREADS 16

/* If an instrumented function is called more than MAX_BLACKLIST_CALLS,
 * then it is added to a blacklist and no longer instrumented.  This
 * is essential for C++ performance. */
#define MAX_BLACKLIST_CALLS 9999

typedef struct TPRecord_ {
    char name[NAME_SIZE];
    void *function_address;
    unsigned int count;
    unsigned int end_count;
    TIME total;
    TIME start;
    TIME max;
    TIME min;
    long long start_mem; /* RAM in use at start */
    long long total_dmem; /* Total change in RAM usage */
    struct TPRecord_ *next;
    struct TPRecord_ *children;
} TPRecord;

typedef struct {
    TPRecord *stack[STACK_SIZE];
    int stack_free;
    TPRecord heap[HEAP_SIZE];
    int heap_free;
    TPRecord *top; /* always points to stack[0] */
} TPContext;

typedef struct {
    TPContext context[MAX_NUM_THREADS];
    int context_free;
} TPGlobalContext;

extern int blacklist[65536];

/* Get the context for the current thread. */
TPContext *TPGetContext();

void TPStartWithContext(TPContext *ctx, const char *name, void *func, long long memory);
void TPEndWithContext(TPContext *ctx, const char *name, void *func, long long memory);
TPRecord *TPFindOrCreate(TPContext *ctx, const char *name, void *func);
void TPPush(TPContext *ctx, TPRecord *r);
TPRecord *TPPop(TPContext *ctx);

#endif /* TIMEPRO_PRIVATE_H */
