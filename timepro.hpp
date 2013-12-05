#ifndef TIMEPRO_HPP
#define TIMEPRO_HPP

#include "timepro.h"
#include <string>
using std::string;

/* A scoped timer for C++ */
class TPTimer {
    private:
        string _name;
    public:
        TPTimer(string name);
        ~TPTimer();
};

/* A scoped timer for C++ which also records memory usage */
class TPMemTimer {
    private:
        string _name;
    public:
        TPMemTimer(string name);
        ~TPMemTimer();
};

/* Prints Timepro results in the destructor -
 * use this to print results whenever a scope
 * exists (ie, in the case of multiple return locations)
 */
class TPPrintTimer {
    private:
        FILE *f_;
        string name_;
    public:
        TPPrintTimer(string name, FILE *f = stdout);
        ~TPPrintTimer();
};
#endif // TIMEPRO_HPP
