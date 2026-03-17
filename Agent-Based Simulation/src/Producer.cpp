#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <numeric>

#include "Distributor.h"
#include "Logger.h"
#include "PriceController.h"
#include "Machine.h"
#include "Person.h"
#include "Producer.h"
#include "Product.h"
#include "Sim.h"
#include "Society.h"

Producer::Producer(
        Society * society,
        const std::unordered_set<Product *>& initial_catalog,
        const std::unordered_map<Product *, int>& input_inventory
        ) :
    Firm(society, initial_catalog, input_inventory) {
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
        this->input_inventory[product] =
            (society->get_initial_production()[product] - product->mean_consumption_frequency) * 
            (FIRM_STOCKPILE_DURATION + FIRM_DEMAND_WINDOW_MIN * PRODUCER_INITIAL_INVENTORY_MULT) *
            STARTING_NUM_PEOPLE;
    }
}

void Producer::on_time_step() {
    Firm::on_time_step();
	move_plans_forward_one_step();
    if (plans_in_progress.size()) {
        log_plans();
    }
}

bool Producer::can_produce(Product * product) {
    return catalog.count(product);
}

int Producer::draft_plan(Order * order) {
    for (std::pair<Product * const, double>& input :
            order->product->inputs_per_unit) {
        if (input_inventory[input.first] < input.second * order->quantity) {
			return DRAFT_ORDER_REJECTED;
        }
    }
	Plan * draft_plan = new Plan{};
	draft_plan->order = order;
	draft_plan->firm = this;
	draft_optimal_plan(draft_plan, order->product->required_abilities);

    if (draft_plan->workers.empty()) {
        delete draft_plan;
        return DRAFT_ORDER_REJECTED;
    }

    double machinery_cost_per_hour = 0.0;
    for (Machine * machine : machines) {
        machinery_cost_per_hour += machine->price_per_unit / machine->lifetime;
    }
    draft_plan->machinery_cost = machinery_cost_per_hour *
                    (static_cast<double>(draft_plan->labor_hours) /
                     draft_plan->workers.size());

	order_to_draft_plan[order] = draft_plan;
    log_draft_plan(draft_plan);
	return draft_plan->predicted_turnaround_time;
}

void Producer::drop_order(Order * order) {
    log_dropped_order(order);
	order_to_draft_plan.erase(order);
}

bool Producer::pursue_order(Order * order) {
	if (!order_to_draft_plan.count(order)) {
		return false;
	}
	Plan * draft_plan = order_to_draft_plan[order];

    for (std::pair<Product * const, double>& input :
            order->product->inputs_per_unit) {
        add_demand_signal(input.first, input.second * order->quantity);
    }
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
	for (std::pair<Product * const, double>& input :
            plan->order->product->inputs_per_unit) {
		remove_input_inventory(input.first, input.second * plan->order->quantity);
	}
    pooled_input_value_account += plan->raw_materials;
    plan->raw_materials = 0;
    plan->total_hours_remaining -= 1e-6;
}

void Producer::move_plan_forward_one_step(Plan * plan) {
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
        // Consume this plan's remaining raw-material value in proportion
        // to productive labor completed this step.
        if (labor_hours_done > 0 && plan->labor_hours_remaining > 0) {
            raw_materials_used =
                pooled_input_value_account *
                static_cast<double>(labor_hours_done) /
                plan->labor_hours_remaining;
        }
    }
	//pay workers
	for (Person * worker : plan->workers) {
		worker->register_hours_worked((double) labor_hours_done / plan->workers.size());
	}
    plan->labor_hours_remaining -= labor_hours_done;
    plan->raw_materials_remaining = std::max(
            0.0,
            plan->raw_materials_remaining - raw_materials_used
            );
    for (std::pair<Product *const, int>& input : input_inventory) {
        if (input.second < RAW_MATERIAL_THRESHOLD) {
            reorder_raw_materials(input.first);
        }
    }
    plan->total_hours_remaining =
        plan->labor_hours_remaining + plan->raw_materials_remaining;
}

double Producer::get_input_products_account() {
    return pooled_input_value_account;
}

int Producer::get_pending_input_inventory(Product * product) {
    int pending_inventory = input_inventory[product];
    for (Order * order : product_to_outbound_orders[product]) {
        pending_inventory += order->quantity;
    }
    return pending_inventory;
}

void Producer::reorder_raw_materials(Product * product) {
    const int reorder_quantity =
        RAW_MATERIAL_THRESHOLD * RAW_MATERIAL_ORDER_MULTIPLIER;
    const double transaction_value =
        product->price_per_unit * reorder_quantity;
    for (Producer * producer : society->get_producers()) {
        if (producer == this) {
            continue;
        }
        if (producer->get_inventory(product) <=
                RAW_MATERIAL_THRESHOLD * RAW_MATERIAL_SURPLUS_FACTOR) {
            continue;
        }
        if (pooled_input_value_account < transaction_value) {
            continue;
        }
        if (producer->sell_input_inventory(
                    product,
                    reorder_quantity,
                    transaction_value
                    )) {
            add_input_inventory(product, reorder_quantity);
            pooled_input_value_account -= transaction_value;
            break;
        }
    }
}

bool Producer::sell_input_inventory(
        Product * product,
        int quantity,
        double transaction_value
        ) {
    if (!remove_input_inventory(product, quantity)) {
        return false;
    }
    register_input_sale(transaction_value);
    return true;
}

void Producer::register_input_sale(double transaction_value) {
    if (plans_in_progress.empty()) {
        return;
    }
    plans_in_progress.front()->prd += transaction_value;
}

void Producer::end_plan(Plan * plan) {
    log_ended_plan(plan);
    plan->order->status = Order::ORDER_FINISHED;
	// simplification: whole product amount is added to inventory at the end of
    // a plan
	input_inventory[plan->order->product] += plan->order->quantity;
	// simplification: product shipped instantly
	input_inventory[plan->order->product] -= plan->order->quantity;
    plan->order->customer->receive_shipment(plan->order);
    double total = plan->order->product->price_per_unit * plan->order->quantity;
    plan->prd += total;
    PriceController::get_instance()->update_price(plan);
    
    for (Person * worker : plan->workers) {
        Society::get_instance()->get_unemployed_people().push_back(worker);
    }
}

void Producer::move_plans_forward_one_step() {
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
			Sim::get_current_time_step() % DAY < Society::get_instance()->get_current_work_hours_daily() && 
			Sim::get_current_time_step() / DAY % 7 < Society::get_instance()->get_current_work_days_weekly()) {
			move_plan_forward_one_step(plan);
		}
		if (plan->total_hours_remaining < 1e-6) {
			end_plan(plan);
			iter = plans_in_progress.erase(iter);
			--iter; 
		}
	}
}

std::unordered_set<Product *> Producer::get_products_to_reorder() {
    std::unordered_set<Product *> products_to_reorder;
    for (Product * product : catalog) {
        for (std::pair<Product * const, double>& input :
                product->inputs_per_unit) {
            products_to_reorder.insert(input.first);
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

void Producer::log_ended_plan(const Plan * plan) {
    Logger::get_instance()->log(
            Logger::PRODUCER,
            "ended_plan",
            id,
            plan->order->product->product_name,
            plan->order->quantity
            );
}
