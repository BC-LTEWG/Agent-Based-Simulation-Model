#include <iostream>
#include <climits>
#include "Distributor.h"
#include "Machine.h"
#include "Person.h"
#include "Producer.h"
#include "Product.h"
#include "Society.h"

Distributor::Distributor(Society * society) : Firm{society} {}

Distributor::Distributor(
        Society * society,
        std::unordered_set<Product *> initial_catalog
        ) :
    Firm(society, initial_catalog)
{
    for (Product * product : get_products_to_reorder()) {
        society->add_consumer_good(product);
        int quantity =
            get_reorder_threshold(product) * FIRM_INITIAL_INVENTORY_MULTIPLIER;
        Order * order = new Order;
        order->product = product;
        order->quantity = quantity;
        order->customer = this;
        order->requested_turnaround_time = 0;
        order->status = Order::ORDER_REQUESTED;
        Plan * plan = new Plan;
        plan->order = order;
        plan->firm = this;
        plan->training_time_remaining = plan->training_time = 0;
        plan->predicted_turnaround_time = 0;
        plan->labor_hours = DISTRIBUTION_LABOR_PER_UNIT * quantity;
        plan->raw_materials_remaining = plan->raw_materials = 0.0;
        plan->total_hours_remaining = plan->total_hours =
            plan->labor_hours + plan->raw_materials;
        plan->prd = -(plan->total_hours);
        plan->outgoing_units_consumed = 0;
        plan->m = 0;
        plans_in_progress.push_back(plan);
        product_to_plan[product] = plan;
        inventory[product] = quantity;
    }
}

void Distributor::on_time_step() {
    Firm::on_time_step();
    for (Plan * plan : plans_in_progress) {
        plan->labor_hours_remaining -= plan->workers.size();
        for (Person * worker : plan->workers) {
            worker->register_hours_worked(1);
        }
    }
}

double Distributor::get_output_ratio(Product& product) {
    return 1.0 / product.order_size;
}

double Distributor::planned_satisfaction_per_person(Product& product, Person& person) {
    return get_output_ratio(product) * person.get_purchase_frequencies()[&product];
}

void Distributor::sell_goods(Product& product, int quantity, Person * person) {
    add_demand_signal(&product, quantity);

    if (!inventory[&product]) {
        std::cerr << "Inventory has no such product: " << product.product_name << std::endl;
        return;
    }
    int available = inventory[&product];
    if (available < quantity) {
        std::cerr << "Shortfall in product " << product.product_name 
            << " of " << quantity - available << " units. " << std::endl;
        return;
    }
    double cost = quantity * product.price_per_unit;
    if (!person->charge(cost)) {
        std::cerr << "Person cannot afford " << quantity 
            << " units of " << product.product_name << " costing " 
            << cost << std::endl;
        return;
    } 
    Plan * plan = product_to_plan[&product];
    plan->outgoing_units_consumed += quantity;
    product_to_plan[&product]->prd += cost;
    inventory[&product] -= quantity;
}

bool Distributor::is_overproduced(Product* product) {
    for(auto& goods : plans_in_progress) {
        if(goods->order->product == product) {
            return goods->order->quantity > PRODUCTION_THRESHOLD * goods->order->quantity;
        }
    }
    return false;
}

std::unordered_set<Product *> Distributor::get_products_to_reorder() {
    return catalog;
}
