/* This is a dummy header for timepro.
 * If USE_TIMEPRO is defined, then it will
 * include the real timepro header */
#ifndef TIMEPRO_HPP
#define TIMEPRO_HPP

#include "timepro.h"
#include <string>
#include <stdio.h>

#ifdef USE_TIMEPRO

#include <timepro_real.hpp>

#else

#define TPTimer std::string
#define TPMemTimer std::string

class TPPrintTimer {
    public:
        TPPrintTimer(std::string name, FILE *f = stdout) {}
};

#endif // USE_TIMEPRO

#endif // TIMEPRO_HPP
