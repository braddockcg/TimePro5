#ifndef TIMEPRO_REAL_HPP
#define TIMEPRO_REAL_HPP

#include <timepro_real.h>

/* A scoped timer for C++ */
class TPTimer {
    private:
        std::string _name;
    public:
        TPTimer(std::string name);
        ~TPTimer();
};

/* A scoped timer for C++ which also records memory usage */
class TPMemTimer {
    private:
        std::string _name;
    public:
        TPMemTimer(std::string name);
        ~TPMemTimer();
};

/* Prints Timepro results in the destructor -
 * use this to print results whenever a scope
 * exists (ie, in the case of multiple return locations)
 */
class TPPrintTimer {
    private:
        FILE *f_;
        std::string name_;
    public:
        TPPrintTimer(std::string name, FILE *f = stdout);
        ~TPPrintTimer();
};

#endif /* TIMEPRO_REAL_HPP */
