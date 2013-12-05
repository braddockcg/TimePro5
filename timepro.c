#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "timepro.h"
#include "timepro_private.h"

TPGlobalContext timepro_global;
int timepro_global_enabled = 0;
int blacklist[65536];

pthread_mutex_t timepro_global_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_once_t timepro_global_once = PTHREAD_ONCE_INIT;

/* Thread local storage of context structure */
pthread_key_t tls_key;

/* This will only be called once per process */
void TPGlobalInit()
{
    pthread_mutex_lock(&timepro_global_mutex);
    for (int i=0; i<65536; ++i)
        blacklist[i] = 0;
    timepro_global.context_free = 0;
    timepro_global_enabled = 1;
    pthread_key_create(&tls_key, 0x0);
    pthread_mutex_unlock(&timepro_global_mutex);
}

void TPInit()
{
    /* Ensures that the global struct is only inited once */
    pthread_once(&timepro_global_once, TPGlobalInit);
}

TPRecord *TPRecordAlloc(TPContext *ctx, const char *name, void *function_address)
{
    TPRecord *r;
    assert(ctx->heap_free < HEAP_SIZE - 1);
    r = &ctx->heap[ctx->heap_free];
    ++ctx->heap_free;
    if (name != 0x0) {
        strncpy(r->name, name, NAME_SIZE);
    } else {
        r->name[0] = 0x0;
    }
    r->function_address = function_address;
    r->count = 0;
    r->end_count = 0;
    r->total = 0;
    r->start = 0;
    r->max = 0;
    r->min = 0;
    r->next = 0x0;
    r->children = 0x0;
    r->start_mem = -1;
    r->total_dmem = 0;
    return r;
}

/* Allocates a TPContext from the global pool */
TPContext *TPAllocContext()
{
    TPContext *c;
    pthread_mutex_lock(&timepro_global_mutex);
    assert(timepro_global.context_free < MAX_NUM_THREADS);
    c = &timepro_global.context[timepro_global.context_free];
    ++timepro_global.context_free;
    pthread_mutex_unlock(&timepro_global_mutex);
    return c;
}

/* Add a child to the parent's linked list of children */
void TPAddChild(TPRecord *parent, TPRecord *child) {
    TPRecord *children = parent->children;
    parent->children = child;
    child->next = children;
}

/* push record onto stack */
void TPPush(TPContext *ctx, TPRecord *r)
{
    assert(ctx->stack_free < STACK_SIZE);
    ctx->stack[ctx->stack_free] = r;
    ++ctx->stack_free;
}

/* Gets or creates the context for this thread */
TPContext *TPGetContext()
{
    if (!timepro_global_enabled)
        return 0x0;
    TPContext *c = (TPContext *) pthread_getspecific(tls_key);
    TPRecord *r;
    if (c != 0x0)
        return c;
    /* else alloc and init new context */
    c = TPAllocContext();
    pthread_setspecific(tls_key, (const void *) c);
    c->stack_free = 0;
    c->heap_free = 0;
    r = TPRecordAlloc(c, "TOP", 0x0);
    c->top = r;
    TPPush(c, c->top);
    return c;
}

TPRecord *TPPop(TPContext *ctx) {
    TPRecord *r;
    assert(ctx->stack_free > 1); // first position is always "TOP"
    --ctx->stack_free;
    r = ctx->stack[ctx->stack_free];
    return r;
}

/* Find or create a record given the specified name */
TPRecord *TPFindOrCreate(TPContext *ctx, const char *name, void *func) {
    TPRecord *active = ctx->stack[ctx->stack_free - 1];
    for (TPRecord *p = active->children; p != 0x0; p = p->next)
    {
        if (name && !strncmp(name, p->name, NAME_SIZE))
            return p;
        if (func != 0x0 && (p->function_address == func))
            return p;
    }
    TPRecord *new = TPRecordAlloc(ctx, name, func);
    TPAddChild(active, new);
    return new;
}

void TPStartWithContext(TPContext *ctx, const char *name, void *func, long long memory) {
    TPRecord *r;
    r = TPFindOrCreate(ctx, name, func);
    ++(r->count);
    r->start = GetTime();
    r->start_mem = memory;
    TPPush(ctx, r);

#if 0
    if (ctx->stack_free > 1) { // sanity check
        TPRecord *r1 = ctx->stack[ctx->stack_free -1];
        TPRecord *r2 = ctx->stack[ctx->stack_free -2];
        if (r1->name[0] && !strncmp(r1->name, r2->name, NAME_SIZE)) {
            fprintf(stderr, "Recursion detected in TPStart %llu vs %llu named %s = %s\n", 
                    (unsigned long long) r1, (unsigned long long)r2, r1->name, r2->name);
            TPPrintStack(ctx);
        }
    }
#endif
}

void TPPrintStack(TPContext *ctx) {
    for (int i=0; i < ctx->stack_free; ++i) {
        if (ctx->stack[i]->function_address) {
            char funcname[NAME_SIZE];
            GetSymbolName(ctx->stack[i]->function_address, funcname, NAME_SIZE);
            fprintf(stderr, "%i: %s\n", i, funcname);
        } else {
            fprintf(stderr, "%i: %s\n", i, ctx->stack[i]->name);
        }
    }
}

void TPPrintBacktrace() {
    TPContext *ctx = TPGetContext();
    if (ctx)
        TPPrintStack(ctx);
}

void TPEndWithContext(TPContext *ctx, const char *name, void *func, long long memory) {
    TPRecord *active = ctx->stack[ctx->stack_free - 1];
    TIME t = GetTime();
    TIME duration;
    if (name != 0x0) // check that our TPEnd matches TPStart
        assert(!strncmp(name, active->name, NAME_SIZE));
    if (func != 0x0 && func != active->function_address) {
        char funcname[NAME_SIZE], funcname2[NAME_SIZE];
        GetSymbolName(func, funcname, NAME_SIZE);
        GetSymbolName(active->function_address, funcname2, NAME_SIZE);
        fprintf(stderr, "TIMEPRO WARNING: func != active->function_address for %s, expected %s\n", funcname, funcname2);
        int i;
        for (i=ctx->stack_free - 1; (i > 0) && (ctx->stack[i]->function_address != func); --i)
            ;
        if (ctx->stack[i]->function_address == func) {
            fprintf(stderr, "TIMEPRO: unwinding stack %i frames\n", ctx->stack_free - i);
            ctx->stack_free = i + 1;
        } else {
            fprintf(stderr, "TIMEPRO: failed to find match in stack\n");
            TPPrintStack(ctx);
            return;
        }
    }
    duration = t - active->start;
    active->total += duration;
    active->total_dmem += memory - active->start_mem;
    ++active->end_count;
    if (active->count == 1 || (duration < active->min))
        active->min = duration;
    if (duration > active->max)
        active->max = duration;
    active->start = 0;
    TPPop(ctx);
}

void TPStart(const char *name) {
    TPContext *ctx = TPGetContext();
    if (ctx)
        TPStartWithContext(ctx, name, 0x0, -1);
}

void TPStartMem(const char *name) {
    TPContext *ctx = TPGetContext();
    if (ctx)
        TPStartWithContext(ctx, name, 0x0, (long long) GetMemUsageFromProc());
}

void TPEnd(const char *name) {
    TPContext *ctx = TPGetContext();
    if (ctx)
        TPEndWithContext(ctx, name, 0x0, -1);
}

void TPEndMem(const char *name) {
    TPContext *ctx = TPGetContext();
    if (ctx)
        TPEndWithContext(ctx, name, 0x0, (long long) GetMemUsageFromProc());
}

int TPChildrenCount(TPRecord *r) {
    int c = 0;
    for (TPRecord *p = r->children; p != 0x0; p = p->next) {
        ++c;
    }
    return c;
}

void TPChildrenToArray(TPRecord *r, TPRecord **array, int *count) {
    int n = TPChildrenCount(r);
    assert(count && *count);
    assert(array);
    assert (n < *count);
    *count = n;
    
    int i = 0;
    for (TPRecord *p = r->children; p != 0x0; p = p->next) {
        array[n - i - 1] = p;
        ++i;
    }
}

int TPRecordCompare(const void *a, const void *b) {
    TPRecord *ra = (TPRecord *) a;
    TPRecord *rb = (TPRecord *) b;
    return rb->total - ra->total;
}

void TPPrintRecord(FILE *f, TPRecord *r, int indent, long min_usec) {
    int max_children = 10000;
    TPRecord *children[10000];
    int n = max_children;
    TPChildrenToArray(r, children, &n);
    //qsort(children, n, sizeof(TPRecord *), TPRecordCompare);
    for (int i=0; i<n; ++i) {
        TPRecord *cr = children[i];
        TIME avg;
        if (cr->total / 1000LLU > min_usec)
        {
            if (cr->count != 0)
                avg = cr->total / cr->count;
            else
                avg = 0;
            fprintf(f, "%5llu %5u %5llu %5llu ", 
                    cr->total / 1000000LLU, cr->count, avg / 1000000LLU, cr->max / 1000000LLU);
            if (cr->start_mem < 0) {
                fprintf(f, "  n/a");
            } else {
                fprintf(f, "%5lli", cr->total_dmem / (1024LL*1024LL));
            }
            for (int j=0; j<indent; ++j)
            {
                if (j % 2 == 0)
                    fprintf(f, "| ");
                else
                    fprintf(f, ": ");
            }
            if (cr->name[0] != 0x0) {
                fprintf(f, "%s", cr->name);
            } else {
                char buf[NAME_SIZE];
                int has_name = GetSymbolName(cr->function_address, buf, NAME_SIZE);
                if (has_name)
                    fprintf(f, "%s", buf);
                else
                    fprintf(f, "%5llu", (unsigned long long) cr->function_address);
            }
            fprintf(f, "\n");
            TPPrintRecord(f, cr, indent + 1, min_usec);
        }
    }
}

void TPPrintThread(FILE *f, TPContext *ctx, long min_usec) {
    fprintf(f, "TOTAL     N AVG  MAX  dMEM(MB) NAME\n");
    TPPrintRecord(f, ctx->top, 0, min_usec);
}

void TPPrunedPrint(FILE *f, long min_usec) {
    for (int thread_num = 0; thread_num < timepro_global.context_free; ++thread_num) {
        TPContext *ctx = &timepro_global.context[thread_num];
        fprintf(f, "TIMEPRO RESULTS FOR THREAD #%i\n", thread_num);
        TPPrintThread(f, ctx, min_usec);
    }
}

void TPPrint(FILE *f) {
    TPPrunedPrint(f, 0);
}

void TPClear() {
    for (int thread_num = 0; thread_num < timepro_global.context_free; ++thread_num) {
        TPContext *c = &timepro_global.context[thread_num];
        c->stack_free = 0;
        c->heap_free = 0;
        TPRecord *r = TPRecordAlloc(c, "TOP", 0x0);
        c->top = r;
        TPPush(c, c->top);
    }
}
