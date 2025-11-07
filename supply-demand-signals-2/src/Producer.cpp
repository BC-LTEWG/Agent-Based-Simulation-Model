#include "Producer.h"

Producer::Producer() : Firm() {}

Producer::Producer(std::vector<Machine*> machines, std::vector<Person*> workforce) 
    : Firm(machines, workforce, {}) {}

Producer::Producer(std::vector<Machine*> machines, std::vector<Person*> workforce, std::vector<Plan*> plans) 
    : Firm(machines, workforce, plans) {}

