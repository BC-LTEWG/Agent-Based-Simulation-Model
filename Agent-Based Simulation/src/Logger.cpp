#ifndef LOG_DBG
#define LOG_DBG 0
#endif

#include <iostream>

#include "Firm.h"
#include "Logger.h"
#include "Person.h"
#include "Product.h"
#include "Sim.h"

Logger::Logger() {}

Logger * Logger::get_instance() {
    static Logger * instance = new Logger;
    return instance;
}

void Logger::log_event(std::string message) {
    int time_step = Sim::get_current_time_step();
    if (LOG_DBG) {
        std::cout << "[" << time_step << "] " <<
            message << std::endl;
    }
}

void Logger::log_firm_shipment_received(std::string product_name, int quantity) {
    int time_step = Sim::get_current_time_step();
    if (LOG_DBG) {
        std::cout << "[" << time_step << "] " <<
            "Firm: " <<
            "Shipment received - " <<
            product_name << " - " << quantity << " units" <<
            std::endl;
    } else {
        Logger::get_instance()->firm_shipments_received.push_back({
                time_step,
                product_name,
                quantity
                });
    }
}

void Logger::log_firm_inventory_level(std::string product_name, int level) {
    int time_step = Sim::get_current_time_step();
    if (LOG_DBG) {
        std::cout << "[" << time_step << "] " <<
            "Firm: " <<
            "Inventory level - " <<
            product_name << " - " << level << " units" <<
            std::endl;
    } else {
        Logger::get_instance()->firm_inventory_levels.push_back({
                time_step,
                product_name,
                level
                });
    }
}

void Logger::log_firm_reorder(std::string product_name, int quantity) {
    int time_step = Sim::get_current_time_step();
    if (LOG_DBG) {
        std::cout << "[" << time_step << "] " <<
            "Firm: " <<
            "Reordering - " <<
            product_name << " - " << quantity << " units" <<
            std::endl;
    } else {
        Logger::get_instance()->firm_reorders.push_back({
                time_step,
                product_name,
                quantity
                });

    }
}

void Logger::log_firm_accepted_order(
        std::string product_name,
        int turnaround_time,
        bool accepted
        ) {
    int time_step = Sim::get_current_time_step();
    if (LOG_DBG) {
        std::cout << "[" << time_step << "] " <<
            "Firm: ";
        if (accepted) {
            std::cout << "Order accepted - ";
        } else {
            std::cout << "No producer found - ";
        }
        std::cout <<
            product_name << " - " << turnaround_time <<
            " requested turnaround time" <<
            std::endl;
    } else {
        if (accepted) {
            Logger::get_instance()->firm_accepted_orders.push_back({
                    time_step,
                    product_name,
                    turnaround_time
                    });
        } else {
            Logger::get_instance()->firm_no_producer_found.push_back({
                    time_step,
                    product_name,
                    turnaround_time
                    });
        }
    }

}

void Logger::log_person_payment(int payment) {
    int time_step = Sim::get_current_time_step();
    if (LOG_DBG) {
        std::cout << "[" << time_step << "] " <<
            "Person: " <<
            "Paid " << payment << " hours" <<
            std::endl;
    } else {
        Logger::get_instance()->person_payments.push_back({
                time_step,
                payment
                });
    }
}

void Logger::log_person_shopping(std::string product_name, int quantity) {
    int time_step = Sim::get_current_time_step();
    if (LOG_DBG) {
        std::cout << "[" << time_step << "] " <<
            "Person: " <<
            "Shopping - " <<
            product_name << " - " << quantity << " units" <<
            std::endl;
    } else {
        Logger::get_instance()->person_shopping.push_back({
                time_step,
                product_name,
                quantity
                });
    }
}

void Logger::log_person_state(int age, double account, Person::HealthStatus health_status) {
    const std::string health_status_string[] = { "healthy", "unhealthy" };
    int time_step = Sim::get_current_time_step();
    if (LOG_DBG) {
        std::cout << "[" << time_step << "] " <<
            "Person: " <<
            "Age - " << age <<
            " account - " << account <<
            " HealthStatus - " << health_status_string[health_status] <<
            std::endl;
    } else {
        Logger::get_instance()->person_state.push_back({
                time_step,
                age,
                account,
                health_status
                });
    }
}

void Logger::log_producer_plans(std::vector<Plan *> plans) {
    int time_step = Sim::get_current_time_step();
    if (LOG_DBG) {
        std::cout << "[" << time_step << "] " <<
            "Producer: "
            "Current plans:" << std::endl;
        for (Plan * plan : plans) {
            std::cout << "Plan for order of " << plan->order->quantity << " units of "
                << plan->order->product->product_name << std::endl;
            std::cout << "\tTraining time remaining: " << plan->training_time_remaining 
                << std::endl;
            std::cout << "\tTotal hours: " << plan->total_hours_remaining << "/" << 
                plan->total_hours << std::endl;
            std::cout << "\tLabor hours: " << plan->labor_hours_remaining << "/" << 
                plan->labor_hours << std::endl;
            std::cout << "\tRaw materials: " << plan->raw_materials_remaining << "/" << 
                plan->raw_materials << std::endl;
            std::cout << "\tPredicted turnaround time: " << 
                plan->predicted_turnaround_time << std::endl;
            std::cout << "\tMachine account: " << plan->m << std::endl;
            std::cout << "\tOutgoing units consumed: " << 
                plan->outgoing_units_consumed << std::endl;
        }
    } else {
        for (Plan * plan : plans) {
            Logger::get_instance()->producer_plans.push_back({ time_step, plan });
        }
    }

}

void Logger::log_producer_draft_plan(std::string product_name, int quantity) {
    int time_step = Sim::get_current_time_step();
    if (LOG_DBG) {
        std::cout << "[" << time_step << "] " <<
            "Producer: " <<
            "Draft plan - " << product_name << " - " << quantity << " units" <<
            std::endl;
    } else {
        Logger::get_instance()->producer_draft_plans.push_back({
                time_step,
                product_name,
                quantity
                });
    }
}

void Logger::log_producer_dropped_order(std::string product_name, int quantity) {
    int time_step = Sim::get_current_time_step();
    if (LOG_DBG) {
        std::cout << "[" << time_step << "] " <<
            "Producer: " <<
            "Dropped order - " << product_name << " - " << quantity << " units" <<
            std::endl;
    } else {
        Logger::get_instance()->producer_dropped_orders.push_back({
                time_step,
                product_name,
                quantity
                });
    }
}

void Logger::log_producer_pursued_plan(Plan * draft_plan) {
    int time_step = Sim::get_current_time_step();
    if (LOG_DBG) {
        std::cout << "[" << time_step << "] " <<
            "Producer: " <<
            "Pursuing order - product - " <<
            draft_plan->order->product->product_name << " - " <<
            draft_plan->order->quantity << " units - " <<
            "with " << draft_plan->workers.size() << " workers - " <<
            "in predicted time " << draft_plan->predicted_turnaround_time << " hours" <<
            std::endl;
    } else {
        Logger::get_instance()->producer_pursued_draft_plans.push_back({ time_step, draft_plan });
    }
}
