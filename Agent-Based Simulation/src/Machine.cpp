#include <string>

#include "Machine.h"

Machine::Machine(const std::string & name, int lifetime) 
    : Product{name}, lifetime{lifetime} {}

