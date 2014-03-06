/* This is a dummy header for timepro.
 * If USE_TIMEPRO is defined, then it will
 * include the real timepro header */
#ifndef TIMEPRO_H
#define TIMEPRO_H

#ifdef USE_TIMEPRO

#include <timepro_real.h>

#else

#define TPInit()
#define TPStart(name)
#define TPEnd(name)
#define TPStartMem(name)
#define TPEndMem(name)
#define TPPrint(f)
#define TPPrunedPrint(f, min_usec)
#define TPClear()
#define TPPrintBacktrace()

#endif /* USE_TIMEPRO */

#endif /* TIMEPRO_H */
