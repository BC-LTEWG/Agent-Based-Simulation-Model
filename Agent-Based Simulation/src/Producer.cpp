#include <algorithm>
#include <iostream>

#include "Distributor.h"
#include "Logger.h"
#include "PriceController.h"
#include "Machine.h"
#include "Person.h"
#include "Producer.h"
#include "Product.h"
#include "Sim.h"
#include "Society.h"

Producer::Producer(Society * society) : Firm{society} {}

Producer::Producer(
        Society * society,
        std::unordered_set<Product *> initial_catalog
        ) :
    Firm(society, initial_catalog)
{
    std::unordered_set<Machine *> initial_machines;
    for (Product * product : initial_catalog) {
        for (Machine * machine : product->machines_needed) {
            initial_machines.insert(machine);
        }
    }
    for (Machine * machine : initial_machines) {
        machines.push_back(machine);
    }
    for (Product * product : get_products_to_reorder()) {
        inventory[product] =
            get_reorder_threshold(product) * FIRM_INITIAL_INVENTORY_MULTIPLIER;
    }
}

void Producer::on_time_step() {
    Firm::on_time_step();
	execute_plans();
    if (plans_in_progress.size()) {
        log_plans();
    }
}

bool Producer::can_produce(Product * product) {
    return catalog.count(product);
}

int Producer::draft_plan(Order * order) {
    bool enough_inputs = true;
    for (auto &p : order->product->inputs_per_unit) {
        if (inventory[p.first] < p.second * order->quantity) {
            enough_inputs = false;
        }
    }
	if (!enough_inputs || order_to_draft_plan[order] != nullptr) {
		return DRAFT_ORDER_REJECTED;
	}
	if (!can_produce(order->product)) {
		catalog.insert(order->product);
	}
	Plan * draft_plan = new Plan();
	draft_plan->order = order;
	draft_plan->firm = this;
	draft_optimal_plan(draft_plan, order->product->required_abilities);

    double machinery_cost_per_hour = 0.0;
    for (Machine * machine : machines) {
        machinery_cost_per_hour += machine->price_per_unit / machine->lifetime;
    }
    if (!draft_plan->workers.empty()) {
        draft_plan->machinery_cost = machinery_cost_per_hour *
                        (static_cast<double>(draft_plan->labor_hours) /
                         draft_plan->workers.size());
    } else {
        draft_plan->machinery_cost = 0.0;
    }

	order_to_draft_plan[order] = draft_plan;
    log_draft_plan(draft_plan);
	return draft_plan->predicted_turnaround_time;
}

bool Producer::drop_order(Order * order) {
    log_dropped_order(order);
	if (order_to_draft_plan[order] == nullptr) {
		return false;
	}
	order_to_draft_plan[order] = nullptr;
	return true;
}

bool Producer::pursue_order(Order * order) {
    for (std::pair<Product *, double> p : order->product->inputs_per_unit) {
        add_demand_signal(p.first, p.second * order->quantity);
    }
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
		it = std::find(
                society->get_unemployed_people().begin(),
                society->get_unemployed_people().end(),
                worker
                );
		if (it != society->get_unemployed_people().end()) {
			society->get_unemployed_people().erase(it);
		}
	}
	// move draft_plan to plans_in_progress
	order_to_draft_plan[order] = nullptr;
	plans_in_progress.push_back(draft_plan);
    log_pursued_plan(draft_plan);
	return true;
}

void Producer::start_plan(Plan * plan) {
	// simplification: consume all raw materials at start of plan
	for (auto &p : plan->order->product->inputs_per_unit) {
		inventory[p.first] -= p.second * plan->order->quantity;
	}
}

void Producer::execute_plan(Plan * plan) {
	int labor_hours_done =
        std::min((int) plan->workers.size(), plan->labor_hours_remaining);
	double raw_materials_used = 0.0; 
	if (plan->training_time_remaining > 0) {
		plan->training_time_remaining--;
		if (plan->training_time_remaining == 0) {
            train_workers(
                    plan->workers,
                    plan->order->product->required_abilities
                    );
        }
	} else {
		raw_materials_used =
            plan->raw_materials *
            labor_hours_done /
            (plan->labor_hours - plan->workers.size() * plan->training_time);
	}
	//pay workers
	for (Person * worker : plan->workers) {
		worker->register_hours_worked((double) labor_hours_done / plan->workers.size());
	}
	plan->labor_hours_remaining -= labor_hours_done;
	plan->raw_materials_remaining -= raw_materials_used;
	plan->total_hours_remaining -= labor_hours_done + raw_materials_used;
}

void Producer::end_plan(Plan * plan) {
    plan->order->status = Order::ORDER_FINISHED;
	// simplification: whole product amount is added to inventory at the end of
    // a plan
	inventory[plan->order->product] += plan->order->quantity;
	// simplification: product shipped instantly
	inventory[plan->order->product] -= plan->order->quantity;
	plan->order->customer->receive_shipment(plan->order);
    plan->prd += plan->order->product->price_per_unit * plan->order->quantity;
    PriceController::update_price(plan);
}

void Producer::execute_plans() {
	for (
            auto iter = plans_in_progress.begin();
            iter != plans_in_progress.end();
            ++iter
            ) {
		Plan * plan = *iter;
		if (plan->total_hours == plan->total_hours_remaining) {
			start_plan(plan);
		}
		if (plan->total_hours_remaining > 0 &&
			Sim::get_current_time_step() % DAY < society->get_current_work_hours_daily() && 
			Sim::get_current_time_step() / DAY % 7 < society->get_current_work_days_weekly()) {
			execute_plan(plan);
		}
		if (plan->total_hours_remaining == 0) {
			end_plan(plan);
			iter = plans_in_progress.erase(iter);
			--iter; 
		}
	}
}

std::unordered_set<Product *> Producer::get_products_to_reorder() {
    std::unordered_set<Product *> products_to_reorder;
    for (Product * product : catalog) {
        for (auto &p : product->inputs_per_unit) {
            products_to_reorder.insert(p.first);
        }
    }
    return products_to_reorder;
}

void Producer::log_plans() {
    for (Plan * plan : plans_in_progress) {
        Logger::get_instance()->log(
                Logger::PRODUCER,
                "plan",
                id,
                plan->order->product->product_name,
                plan->order->quantity
                );
    }
}

void Producer::log_draft_plan(const Plan * draft_plan) {
    Logger::get_instance()->log(
            Logger::PRODUCER,
            "draft_plan",
            id,
            draft_plan->order->product->product_name,
            draft_plan->order->quantity
            );
}

void Producer::log_dropped_order(const Order * order) {
    Logger::get_instance()->log(
            Logger::PRODUCER,
            "dropped_order",
            id,
            order->product->product_name,
            order->quantity
            );
}

void Producer::log_pursued_plan(const Plan * draft_plan) {
    Logger::get_instance()->log(
            Logger::PRODUCER,
            "pursued_plan",
            id,
            draft_plan->order->product->product_name,
            draft_plan->order->quantity
            );
}

