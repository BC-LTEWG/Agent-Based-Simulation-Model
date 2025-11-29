#include "Firm.h"

Firm::Firm() {}

Firm::Firm(std::vector<Machine*> machines, std::vector<Person*> workforce, std::vector<Plan*> plans) 
    : machines(machines), workers(workforce), plans(plans) {}

double Firm::get_avg_productivity() {
    return 0.0;
}

int Firm::assign_workers(std::vector<Ability>& needed_abilities, int num_workers, Plan& plan) {
	return 0;
}

double Firm::suitability(std::vector<Ability>& needed_abilities, Person * person) {
	double suitability = 1.0;
	for (Ability ability : needed_abilities) {
		suitability *= person->get_abilities()[ability];
	}
	return suitability;
}

