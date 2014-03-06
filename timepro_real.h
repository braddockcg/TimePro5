#ifndef TIMEPRO_REAL_H
#define TIMEPRO_REAL_H

/*check if the compiler is of C++*/
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/* TPInit() must be called once per process before any other Timepro calls
 * are made */
void TPInit();

/* Start a named interval timer */
void TPStart(const char *name);

/* End a named interval */
void TPEnd(const char *name);

/* Start a named interval timer and record memory usage (15 microsec overhead)*/
void TPStartMem(const char *name);

/* End a named interval and record memory usage */
void TPEndMem(const char *name);

/* Print results to a file handle.
 * TPPrint is not thread safe.  All other threads should be inactive
 * when TPPrint() is called */
void TPPrint(FILE *f);

/* Print results, but eliminate any functions with
 * a total time of less than min_usec microseconds */
void TPPrunedPrint(FILE *f, long min_usec);

/* Clear all statistics.
 * TPClear is not thread safe.  All other threads should be inactive
 * when TPClear() is called */
void TPClear();

void TPPrintBacktrace();

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* TIMEPRO_REAL_H */
