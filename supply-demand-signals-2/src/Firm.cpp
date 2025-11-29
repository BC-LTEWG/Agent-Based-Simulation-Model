#include "Firm.h"

Firm::Firm() {}

Firm::Firm(std::vector<Machine*> machines, std::vector<Person*> workforce, std::vector<Plan*> plans) 
    : machines(machines), workers(workforce), plans(plans) {}

double Firm::get_avg_productivity() {
    return 0.0;
}

int Firm::assign_workers(std::vector<Ability>& required_abilities, Plan& plan, bool is_query) {
	// option 1 : use a suitability threshold to pick good fit workers
	std::vector<int> option_1_from_curr_workforce, option_1_from_unemployed_pool;
	double option_1_total_suitability;
	assign_workers_by_suitability(required_abilities, plan, SUITABILITY_THRESHOLD, option_1_from_curr_workforce, option_1_from_unemployed_pool, option_1_total_suitability);

	// option 2 : take workers that have any of the required_abilities at all
	std::vector<int> option_2_from_curr_workforce, option_2_from_unemployed_pool;
	double option_2_total_suitability;
	assign_workers_by_suitability(required_abilities, plan, 0.0, option_2_from_curr_workforce, option_2_from_unemployed_pool, option_2_total_suitability);

	// choose best option
	std::vector<int> from_curr_workforce, from_unemployed_pool;
	double total_suitability;
	if (predict_completion_time(plan, option_1_total_suitability) <= plan.allotted_time || 
		predict_completion_time(plan, option_1_total_suitability) < predict_completion_time(plan, option_2_total_suitability)) {
		from_curr_workforce = option_1_from_curr_workforce;
		from_unemployed_pool = option_1_from_unemployed_pool;
		total_suitability = option_1_total_suitability;
	} else {
		from_curr_workforce = option_2_from_curr_workforce;
		from_unemployed_pool = option_2_from_unemployed_pool;
		total_suitability = option_2_total_suitability;
	}

	if (!is_query) {
		reverse(from_curr_workforce.begin(), from_curr_workforce.end());
		reverse(from_unemployed_pool.begin(), from_unemployed_pool.end());
		for (int &i : from_curr_workforce) {
			plan.workers.push_back(workers[i]);
			workers.erase(workers.begin() + i);
		}
		for (int &i : from_unemployed_pool) {
			plan.workers.push_back(Society::instance->unemployed_people[i]);
			Society::instance->unemployed_people.erase(Society::instance->unemployed_people.begin() + i);
		}
	}

	return predict_completion_time(plan, total_suitability);
}

double Firm::suitability(std::vector<Ability>& required_abilities, Person * person) {
	double suitability = 0.0;
	for (Ability ability : required_abilities) {
		suitability += person->get_abilities()[ability];
	}
	suitability /= required_abilities.size();
	return suitability;
}

double Firm::predict_completion_time(Plan& plan, double total_suitability) {
	return plan.total_orders * plan.product->living_labor_per_order * DAY / total_suitability / Society::instance->current_work_hours_daily;
}

double Firm::predict_completion_time(Plan& plan, std::vector<Ability>& required_abilities) {
	double total_suitability = 0.0;
	for (Person * worker : plan.workers) {
		total_suitability += suitability(required_abilities, worker);
	}
	return predict_completion_time(plan, total_suitability);
}


void Firm::assign_workers_by_suitability(std::vector<Ability>& required_abilities, Plan& plan, double suitability_threshold, std::vector<int>& from_curr_workforce, std::vector<int>& from_unemployed_pool, double& total_suitability) {
	total_suitability = 0.0;
	sort(workers.begin(), workers.end(), [&](Person * a, Person * b) {
		return suitability(required_abilities, a) > suitability(required_abilities, b); 
	});
	for (int i = 0; i < workers.size() &&
					predict_completion_time(plan, total_suitability) > plan.allotted_time && 
					suitability(required_abilities, workers[i]) > suitability_threshold; i++) {
		from_curr_workforce.push_back(i);
		total_suitability += suitability(required_abilities, workers[i]);
	}
	for (int i = 0; i < Society::instance->unemployed_people.size() && 
					predict_completion_time(plan, total_suitability) > plan.allotted_time; i++) {
		if (suitability(required_abilities, Society::instance->unemployed_people[i]) > suitability_threshold) {
			from_unemployed_pool.push_back(i);
			total_suitability += suitability(required_abilities, Society::instance->unemployed_people[i]);
		}
	}
}

