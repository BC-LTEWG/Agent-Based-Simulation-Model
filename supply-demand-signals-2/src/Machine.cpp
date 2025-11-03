#include <string>

#include "Machine.h"

Machine::Machine(const std::string& name, int lifetime) 
    : name(name), lifetime(lifetime), hours_used(0) {}

int Machine::get_hours_used() {
    return hours_used;
}
