#include <string>

#include "Machine.h"

Machine::Machine(const std::string& name, int lifetime) 
    : Product{name, 0, 0}, lifetime(lifetime), hours_used(0) {}

