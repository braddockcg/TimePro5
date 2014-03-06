#include "timepro.hpp"

TPTimer::TPTimer(std::string name) 
{
    _name = name;
    TPStart(name.c_str());
}

TPTimer::~TPTimer() {
    TPEnd(_name.c_str());
}

TPMemTimer::TPMemTimer(std::string name) 
{
    _name = name;
    TPStartMem(name.c_str());
}

TPMemTimer::~TPMemTimer() {
    TPEndMem(_name.c_str());
}

TPPrintTimer::TPPrintTimer(std::string name, FILE *f) {
    f_ = f;
    name_ = name;
    TPStartMem(name_.c_str());
}

TPPrintTimer::~TPPrintTimer() {
    TPEndMem(name_.c_str());
    TPPrint(f_);
}

