#include <string>

#include "Machine.h"

Machine::Machine(const std::string& name, int lifetime, double cost_of_machine) 
    : Product{name}, lifetime{lifetime}, cost_of_machine{cost_of_machine} {}

