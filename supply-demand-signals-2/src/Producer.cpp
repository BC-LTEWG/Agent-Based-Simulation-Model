#include "Distributor.h"
#include "Producer.h"
#include "Product.h"
#include "Society.h"

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
	draft_plan->order = order;
	draft_plan->firm = this;
	assign_workers(draft_plan, order->product->required_abilities);
	
	// incomplete
	draft_plan->prd = 0;
	draft_plan->labor_hours = draft_plan->labor_hours_remaining = 0;
	draft_plan->raw_materials = draft_plan->raw_materials_remaining = 0;
	draft_plan->total_hours = draft_plan->total_hours_remaining = 0; 

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

void Producer::execute_plans() {
	for (auto iter = plans_in_progress.begin(); iter != plans_in_progress.end(); ++iter) {
		Plan * plan = *iter;
		if (plan->total_hours_remaining > 0) {
			int work_done = std::min((int) plan->workers.size(), plan->total_hours_remaining);
			plan->total_hours_remaining -= work_done;
			plan->labor_hours_remaining -= work_done;
			// nothing on raw materials yet
			// nor paying workers
		}
		if (plan->total_hours_remaining == 0) {
			// whole product amount is added to inventory at the end of a plan, not throughout
			int units_produced = plan->order->quantity * plan->order->product->order_size;
			inventory[plan->order->product] += units_produced;
			// simplification: shipped instantly
			inventory[plan->order->product] -= units_produced;
			plan->order->customer->receive_shipment(plan->order->product, units_produced);
		}
	}
}
