#define _GNU_SOURCE
#include <dlfcn.h> // dladdr
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "platform.h"

TIME GetTime() {
    struct timespec ts;
    TIME t;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    t = 1000000000ULL * ts.tv_sec  + ts.tv_nsec; // / 1000ULL;
    return t;
}

int Demangle(const char *mangled_name, char *output, int output_len) {
    size_t n;
    int status;
    char *buf = __cxa_demangle(mangled_name, 0x0, &n, &status);
    if (status == 0) { /* copy buf to output */
        strncpy(output, buf, output_len);
        free(buf);
    } else {
        strncpy(output, mangled_name, output_len);
    }
    return status;
}

int GetSymbolName(void *addr, char *output, int output_len) {
    Dl_info info;
    dladdr(addr, &info);
    int status;
    if (info.dli_sname) {

        /* Attempt to demangle if a C++ name */
        Demangle(info.dli_sname, output, output_len);

        return 1;
    } else {
        return 0;
    }
}

void PrintSymbolName(FILE *f, void *addr) {
    char func[150];
    GetSymbolName(addr, func, 150);
    fprintf(stderr, "%s", func);
}

unsigned long long GetMemUsage() {
#if 0
    struct rusage ru;
    int res;
    res = getrusage(RUSAGE_SELF, &ru);
    return ru.ru_maxrss;
#elif 1
    /* This works with malloc, but not with C++ new/delete */
    struct mallinfo mi = mallinfo();
    return mi.hblkhd;
#else
    void *segment = sbrk(0);
    return (unsigned long long) segment;
#endif
}

unsigned long long GetMemUsageFromProc() {
    long long ps = getpagesize();
    FILE *f = fopen("/proc/self/statm", "r");
    long long size, resident, share, text, lib, data, dt;
    fscanf(f, "%lli %lli %lli %lli %lli %lli %lli", 
            &size, &resident, &share, &text, &lib, &data, &dt);
    fclose(f);
    //printf("size:\t %lli\nresident:\t %lli\nshare:\t %lli\ntext:\t %lli\nlib:\t %lli\ndata:\t %lli\ndt:\t %lli\n",
    //        ps*size, ps*resident, ps*share, ps*text, ps*lib, ps*data, ps*dt);
    return (unsigned long long) ps*size;
}

