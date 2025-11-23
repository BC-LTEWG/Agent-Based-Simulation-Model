#include "Firm.h"

Firm::Firm() {}

Firm::Firm(std::vector<Machine*> machines, std::vector<Person*> workforce, std::vector<Plan*> plans) 
    : machines(machines), workers(workforce), plans(plans) {}

double Firm::get_avg_productivity() {
    return 0.0;
}

void Firm::set_society(Society * society) {
	this->society = society;
}
