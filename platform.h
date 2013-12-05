#ifndef TIMER_H
#define TIMER_H

#include <time.h>

typedef unsigned long long TIME;

TIME GetTime();

/* Return a symbol name given a function address.  Only works if a shared
 * library or compiled with --export-dynamic */
int GetSymbolName(void *addr, char *output, int output_len);

/* Friendly C++ name demangle.  If demangle fails, then mangled_name is
 * copied verbatum to output buffer. */
int Demangle(const char *mangled_name, char *output, int output_len);

/* Pretty print a symbol name given a function address */
void PrintSymbolName(FILE *f, void *addr);

/* Documented at http://mentorembedded.github.io/cxx-abi/abi.html#demangler
 * requires -lstdc++.  NOTE: buf MUST be allocated with malloc.  demangle
 * may realloc it - NASTY */
char* __cxa_demangle (const char* mangled_name, char* buf, size_t* n, int* status);

/* Returns memory usage in kb, specifically the maximum resident set size
 * used (in kilobytes) */
unsigned long long GetMemUsage();

unsigned long long GetMemUsageFromProc();

#endif /* TIMER_H */
