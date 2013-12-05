#define _GNU_SOURCE
#include <dlfcn.h> // dladdr

#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <malloc.h>


#include "platform.h"
#include "timepro.h"
#include "timepro_private.h"

#define TIMED_TEST(func1, func2, name, nloop) \
{ \
    struct timespec t0, t1; \
    long long dt0, dt1, dt; \
    /* First loop warms up the CPU */ \
    int x = 0; \
    for (int i=0; i<(nloop); ++i) { \
        x = (x + 127) / 3; \
    } \
    clock_gettime(CLOCK_MONOTONIC_RAW, &t0); \
    x = 0; \
    for (int i=0; i<(nloop); ++i) { \
        x = (x + 127) / 3; \
    } \
    clock_gettime(CLOCK_MONOTONIC_RAW, &t1); \
    dt0 = t1.tv_nsec - t0.tv_nsec; \
    clock_gettime(CLOCK_MONOTONIC_RAW, &t0); \
    x = 0; \
    for (int i=0; i<(nloop); ++i) { \
        x = (x + 127) / 3; \
        (func1); \
        (func2); \
    } \
    clock_gettime(CLOCK_MONOTONIC_RAW, &t1); \
    dt1 = t1.tv_nsec - t0.tv_nsec; \
    dt = dt1 - dt0; \
    printf("%s time per call %lli nanosec\n", (name), dt/(nloop)); \
}


int main(int argc, char **argv);



void test_timer_raw() {
    struct timespec ts, t0, t1;
    int res;
    res = clock_getres(CLOCK_MONOTONIC_RAW, &ts);
    assert(!res);
    printf("MONOTONIC_RAW RES = %lds %ldns\n", ts.tv_sec, ts.tv_nsec);

    res = clock_gettime(CLOCK_MONOTONIC_RAW, &t0);
    assert(!res);
    printf("gettime(MONOTONIC_RAW RES) t0 = %lds %ldns\n", t0.tv_sec, t0.tv_nsec);
    res = clock_gettime(CLOCK_MONOTONIC_RAW, &t1);
    printf("gettime(MONOTONIC_RAW RES) t1 = %lds %ldns\n", t1.tv_sec, t1.tv_nsec);

    usleep(100);
    clock_gettime(CLOCK_MONOTONIC_RAW, &t1);
    printf("gettime(MONOTONIC_RAW RES) t2 = %lds %ldns\n", t1.tv_sec, t1.tv_nsec);
}

void test_timer() {
    TIME t0, t1, t2, t3, t4;
    int nloop = 1000000;
    t0 = GetTime();
    usleep(100);
    t1 = GetTime();
    printf("t0 = %lld, t1 = %lld, dt = %lld\n", t0, t1, t1 - t0);

    t0 = GetTime();
    int x = 0;
    for (int i=0; i<nloop; ++i)
    {
        x = (x + 127) / 3;
    }
    t1 = GetTime();
    printf("Uninstrumented inner loop execution time = %lld usec\n", t1 - t0);

    t0 = GetTime();
    x = 0;
    for (int i=0; i<nloop; ++i)
    {
        x = (x + 127) / 3;
        t4 = GetTime();
    }
    t1 = GetTime();
    printf("Inner loop execution time with GetTime calls = %lld usec = %lld usec avg per call\n", t1 - t0, (t1 - t0)/nloop);

    struct timespec ts;
    TIMED_TEST(clock_gettime(CLOCK_MONOTONIC_RAW, &ts), 0, "clock_gettime", 1000000);
    TIMED_TEST(GetTime(), 0, "GetTime()", 1000000);
}

void empty() {
}

void lock_unlock(pthread_mutex_t *lock) {
    pthread_mutex_lock(lock); 
    pthread_mutex_unlock(lock);
}

void test_locks() {
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    TIMED_TEST(empty(), 0, "NOP Function", 1000000);
    TIMED_TEST(lock_unlock(&lock), 0,"Mutex lock/unlock", 1000000);
}

void test_dladdr() {
    Dl_info info;
    dladdr(test_locks, &info);
    printf("dladdr(%llx) got %s %s\n", (long long unsigned) test_locks, info.dli_fname, info.dli_sname);
}

void test_memusage() {
    int bytes = GetMemUsage();
    printf("Max RSS = %i\n", bytes);
    TIMED_TEST(GetMemUsage(), 0, "GetMemUsage", 1000000);
    struct mallinfo mi = mallinfo();
    printf("uordblks = %d fordblks = %d hblkhd = %d\n", mi.uordblks, mi.fordblks, mi.hblkhd);

    TIMED_TEST(GetMemUsageFromProc(), 0, "GetMemUsageFromProc", 1000);
}

void test_demangle() {
    char buf[100];
    size_t buflen = 100;
    int status = 0;
    __cxa_demangle("_ZNK4Core16ReferenceCounted20releaseWeakReferenceEPNS_11WeakRefBaseE", buf, &buflen, &status);
    printf("Demangled status %i name: %s\n", status, buf);
    buflen = 100;
    __cxa_demangle("main", buf, &buflen, &status);
    printf("Demangled status %i name: %s\n", status, buf);
}

void test_timepro_speed() {
    TPInit();
    TIMED_TEST(TPGetContext(), 0, "TPGetContext()", 10000);
    TPContext *ctx = TPGetContext();
    TIMED_TEST(TPFindOrCreate(ctx, "foo", 0x0), 0, "TPFindOrCreate()", 10000);
    TPRecord *r = TPFindOrCreate(ctx, "foo", 0x0);

    TIMED_TEST(TPPush(ctx, r), 0, "TPPush", 95);
    TIMED_TEST(TPEndWithContext(ctx, "foo", 0x0, 0), 0, "TPEndWithContext", 95);
    
    TIMED_TEST(TPPush(ctx, r), 0, "TPPush", 95);
    TIMED_TEST(TPPop(ctx), 0, "TPPop", 95);

    TIMED_TEST(strncmp("foo", "foo", NAME_SIZE), 0, "strncmp", 95);
    TIMED_TEST(TPStart("hi"), TPEnd("hi"), "TPStart(hi);TPEnd(hi)", 10000);
}

void test_timepro() {
    TPInit();
    TPInit(); // second call should do nothing
    TPStart("hello");
    usleep(10000);
    {
        TPStartMem("a");
        usleep(5000);
        TPStartMem("b");
        usleep(15000);
        printf("PRE MALLOC %llu or %llu\n", GetMemUsage(), GetMemUsageFromProc());
        int n = 20000000;
        printf("malloc(%d)\n", sizeof(int) * n);
        int *mem = (int *) malloc(sizeof(int) * n);
        for (int i=0; i<   n; i += 1000000) {
            *(mem + i) = i;
        }
        printf("POST MALLOC %llu or %llu\n", GetMemUsage(), GetMemUsageFromProc());
        TPEndMem("b");
        free(mem);
        TPEndMem("a");
        TPStart("a");
        usleep(3000);
        TPEnd("a");
    }
    TPEnd("hello");
    TPStart("b");
    TPEnd("b");
    TPPrint(stdout);
    TPClear();
    TPPrint(stdout);
}

int main(int argc, char **argv) {
    test_timer_raw();
    test_timer();
    test_locks();
    test_dladdr();
    test_demangle();
    test_timepro();
    test_timepro_speed();
    test_memusage();
    return 0;
}
