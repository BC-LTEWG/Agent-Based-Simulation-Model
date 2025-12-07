#include "Distributor.h"
#include "Producer.h"
#include "Product.h"
#include "Sim.h"
#include "Society.h"
#include <algorithm>

Producer::Producer() : Firm() {}

void Producer::on_time_step() {
	execute_plans();
}

bool Producer::can_produce(Product * product) {
    return catalog.count(product);
}

int Producer::draft_order(Order * order) {
	if (order_to_draft_plan[order] != nullptr) {
		return DRAFT_ORDER_REJECTED;
	}
	if (!can_produce(order->product)) {
		catalog.insert(order->product);
	}
	Plan * draft_plan = new Plan();
    draft_plan-> m = draft_plan->labor_hours / draft_plan->workers.size();
	draft_plan->order = order;
	draft_plan->firm = this;
	draft_optimal_plan(draft_plan, order->product->required_abilities);

	order_to_draft_plan[order] = draft_plan;
	return draft_plan->predicted_turnaround_time;
}

bool Producer::drop_order(Order * order) {
	if (order_to_draft_plan[order] == nullptr) {
		return false;
	}
	order_to_draft_plan[order] = nullptr;
	return true;
}

bool Producer::pursue_order(Order * order) {
	if (order_to_draft_plan[order] == nullptr) {
		return false;
	}
	Plan * draft_plan = order_to_draft_plan[order];
	// remove all workers from their current pools
	for (Person * worker : draft_plan->workers) {
		auto it = std::find(workers.begin(), workers.end(), worker);
		if (it != workers.end()) {
			workers.erase(it);
		}
		it = std::find(Society::instance->unemployed_people.begin(), Society::instance->unemployed_people.end(), worker);
		if (it != Society::instance->unemployed_people.end()) {
			Society::instance->unemployed_people.erase(it);
		}
	}
	// move draft_plan to plans_in_progress
	order_to_draft_plan[order] = nullptr;
	plans_in_progress.push_back(draft_plan);
	return true;
}

void Producer::start_plan(Plan * plan) {
	// simplification: consume all raw materials at start of plan
	for (auto &p : plan->order->product->inputs_per_order) {
		inventory[p.first] -= p.second * plan->order->quantity;
	}
}

void Producer::execute_plan(Plan * plan) {
	int labor_hours_done = std::min((int) plan->workers.size(), plan->labor_hours_remaining);
	double raw_materials_used = 0.0; 
	if (plan->training_time_remaining > 0) {
		plan->training_time_remaining--;
		if (plan->training_time_remaining == 0) train_workers(plan->workers, plan->order->product->required_abilities);
	} else {
		raw_materials_used = plan->raw_materials * labor_hours_done / (plan->labor_hours - plan->workers.size() * plan->training_time);
	}
	plan->labor_hours_remaining -= labor_hours_done;
	plan->raw_materials_remaining -= raw_materials_used;
	plan->total_hours_remaining -= labor_hours_done + raw_materials_used;
	plan->prd += labor_hours_done + raw_materials_used;
}

void Producer::end_plan(Plan * plan) {
	// simplification: whole product amount is added to inventory at the end of a plan
	int units_produced = plan->order->quantity * plan->order->product->order_size;
	inventory[plan->order->product] += units_produced;
	// simplification: product shipped instantly
	inventory[plan->order->product] -= units_produced;
	plan->order->customer->receive_shipment(plan->order->product, units_produced);
}

void Producer::execute_plans() {
	for (auto iter = plans_in_progress.begin(); iter != plans_in_progress.end(); ++iter) {
		Plan * plan = *iter;
		if (plan->total_hours == plan->total_hours_remaining) {
			start_plan(plan);
		}
		if (plan->total_hours_remaining > 0 && Sim::get_current_time_step() % DAY < Society::instance->current_work_hours_daily) {
			execute_plan(plan);
		}
		if (plan->total_hours_remaining == 0) {
			end_plan(plan);
			iter = plans_in_progress.erase(iter);
			--iter; 
		}
	}
}
