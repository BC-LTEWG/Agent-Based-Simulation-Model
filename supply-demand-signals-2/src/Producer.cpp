#include "Producer.h"
#include "Product.h"
#include "Distributor.h"

Producer::Producer() : Firm() {}

Producer::Producer(std::vector<Machine *> machines, std::vector<Person *> workforce) 
    : Firm(machines, workforce, {}) {}

Producer::Producer(std::vector<Machine *> machines, std::vector<Person *> workforce, std::vector<Plan *> plans) 
    : Firm(machines, workforce, plans) {}

void Producer::on_time_step() {
}

void Producer::add_product_to_catalog(Product * product) {
    products.insert(product);
}

bool Producer::can_produce(Product * product) {
    return products.find(product) != products.end();
}

std::set<Product *>& Producer::get_catalog() {
    return products;
}

Order * Producer::accept_order(Product * product, int quantity, Distributor * customer) {
    if (!can_produce(product)) {
        return nullptr;
    }
    
    Order * order = new Order();
    order->product = product;
    order->quantity = quantity;
    order->customer = customer;
    
    int total_labor_hours = quantity * product->living_labor_per_order;
    int available_workers = workers.size();
    int hours_per_day = 8;
    
    if (available_workers > 0) {
        int days_needed = (total_labor_hours + (available_workers * hours_per_day - 1)) / (available_workers * hours_per_day);
        order->requested_turnaround_time = days_needed;
    } else {
        order->requested_turnaround_time = 999;
    }
    
    pending_orders.push_back(order);
    
    Plan * plan = new Plan();
	plan->order = order;
    plan->firm = this;
    plan->labor_hours = total_labor_hours;
    plan->labor_hours_remaining = total_labor_hours;
    plan->total_hours = total_labor_hours;
    plan->total_hours_remaining = total_labor_hours;
    plan->raw_materials = 0;
    plan->raw_materials_remaining = 0;
    plan->prd = 0;
    
    plans.push_back(plan);
    
    return order;
}

void Producer::process_orders() {
    int available_labor = workers.size() * 8;
    
    for (Plan * plan : plans) {
        if (plan->total_hours_remaining > 0 && available_labor > 0) {
            int work_done = std::min(available_labor, plan->total_hours_remaining);
            plan->total_hours_remaining -= work_done;
            plan->labor_hours_remaining -= work_done;
            available_labor -= work_done;
            
            if (plan->total_hours_remaining == 0) {
                inventory[plan->order->product] += plan->order->quantity * plan->order->product->order_size;
                
                for (auto it = pending_orders.begin(); it != pending_orders.end(); ++it) {
                    Order * order = *it;
                    if (order->product == plan->order->product) {
                        complete_order(order);
                        pending_orders.erase(it);
                        break;
                    }
                }
            }
        }
    }
}

void Producer::complete_order(Order * order) {
    if (inventory[order->product] >= order->quantity) {
        inventory[order->product] -= order->quantity;
        order->customer->receive_shipment(order->product, order->quantity);
    }
}

