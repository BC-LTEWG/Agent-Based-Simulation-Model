#include "Distributor.h"
#include "Firm.h"
#include "Person.h"
#include "Product.h"
#include "Society.h"

Firm::Firm() {}

double Firm::get_avg_productivity() {
    return 0.0;
}

double Firm::suitability(Person * person, std::vector<Ability>& required_abilities) {
	double suitability = 0.0;
	for (Ability ability : required_abilities) {
		suitability += person->get_abilities()[ability];
	}
	suitability /= required_abilities.size();
	return suitability;
}

int Firm::predict_workers_needed(Order * order) {
	return ceil(order->quantity * order->product->living_labor_per_order * DAY / Society::instance->current_work_hours_daily / order->requested_turnaround_time);
}

double Firm::predict_turnaround_time(Order * order, double total_suitability) {
	return order->quantity * order->product->living_labor_per_order * DAY / total_suitability / Society::instance->current_work_hours_daily;
}
		
void Firm::assign_predicted_turnaround_time(Plan * draft_plan, std::vector<Ability>& required_abilities) {
	double total_suitability = 0.0;
	double max_suitability = 0.0;
	for (Person * worker : draft_plan->workers) {
		double worker_suitability = suitability(worker, required_abilities);
		total_suitability += suitability(worker, required_abilities);
		max_suitability = std::max(max_suitability, worker_suitability);
	}
	if (draft_plan->will_train_workers) {
		draft_plan->predicted_turnaround_time = FIRM_TRAINING_TIME + predict_turnaround_time(draft_plan->order, max_suitability * draft_plan->workers.size());
	} else {
		draft_plan->predicted_turnaround_time = predict_turnaround_time(draft_plan->order, total_suitability);
	}
}

void Firm::assign_workers_by_suitability_threshold(Plan * draft_plan, std::vector<Ability>& required_abilities, double suitability_threshold) {
	std::unordered_map<Person *, double> worker_to_suitability;
	for (Person * worker : workers) {
		worker_to_suitability[worker] = suitability(worker, required_abilities);
	}
	sort(workers.begin(), workers.end(), [&](Person * a, Person * b) {
		return worker_to_suitability[a] > worker_to_suitability[b];
	});

	int max_workers = predict_workers_needed(draft_plan->order);
	for (Person * worker : workers) {
		if (draft_plan->workers.size() >= max_workers || worker_to_suitability[worker] <= suitability_threshold) {
			break;
		} 
		draft_plan->workers.push_back(worker);
	}
 	for (Person * unemployed_person : Society::instance->unemployed_people) {	
		if (draft_plan->workers.size() >= max_workers) {
			break;
		} else if (suitability(unemployed_person, required_abilities) > suitability_threshold) {
			draft_plan->workers.push_back(unemployed_person);
		}
	}
}

void Firm::assign_workers(Plan * draft_plan, std::vector<Ability>& required_abilities) {
	// try without training first
	Plan * draft_plan_without_training = new Plan(*draft_plan);
	assign_workers_by_suitability_threshold(draft_plan_without_training, required_abilities, 0.0);
	assign_predicted_turnaround_time(draft_plan_without_training, required_abilities);
	if (draft_plan_without_training->predicted_turnaround_time <= draft_plan->order->requested_turnaround_time) {
		*draft_plan = *draft_plan_without_training; return;
	}
	
	// did not meet the deadline without training
	// pick the better option between training or no training
	Plan * draft_plan_with_training = new Plan(*draft_plan_without_training);
	draft_plan_with_training->will_train_workers = true;
	assign_predicted_turnaround_time(draft_plan_with_training, required_abilities);
	if (draft_plan_without_training->predicted_turnaround_time < draft_plan_with_training->predicted_turnaround_time) {
		*draft_plan = *draft_plan_without_training;
	} else {
		*draft_plan = *draft_plan_with_training;
	}
}







