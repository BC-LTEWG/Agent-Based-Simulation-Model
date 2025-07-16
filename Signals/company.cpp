#include <iostream>
#include <string>

#include "company.h"

Inputs::Inputs() {
    fixed_capital = 0.0;
    raw_materials = 0.0;
    labor = 0.0;
}

Plan::Plan() {
    product_name = "";
    quantity = 0;
    unit = "";
    company = NULL;
    product = 0.0;
}

Company::Company(string name) {
    company_name = name;
}

string Company::name() {
    return company_name;
}

void Company::add_plan(Plan * plan) {
    plan->company = this;
    plan->product = -(plan->inputs.fixed_capital + plan->inputs.raw_materials
            + plan->inputs.labor);
    plans.push_back(plan);
}

void Company::print_plans() {
    for (Plan * plan : plans) {
        cout << "Product: " << plan->product_name << " - " << plan->quantity
            << " " << plan->unit << (plan->quantity > 1 ? "s" : "") << ":" << endl;
        cout << "\tFixed capital: " << plan->inputs.fixed_capital << endl;
        cout << "\tRaw materials: " << plan->inputs.raw_materials << endl;
        cout << "\tLabor: " << plan->inputs.labor << endl;
    }
}

void Company::employ(Worker * worker) {
    workers.push_back(worker);
}
