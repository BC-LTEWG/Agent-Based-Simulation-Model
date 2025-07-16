#include <iostream>
#include <string>

#include "company.h"

Plan::Plan() {
    product_name = "";
    quantity = 0;
    unit = "";
    company = NULL;
    fixed_capital = 0.0;
    raw_materials = 0.0;
    labor = 0.0;
    product = 0.0;
}

Company::Company(string name) {
    company_name = name;
}

string Company::name() {
    return company_name;
}

void Company::add_plan(Plan plan) {
    plan.company = this;
    plan.product = -(plan.fixed_capital + plan.raw_materials + plan.labor);
    plans.push_back(plan);
}

void Company::print_plans() {
    for (Plan plan : plans) {
        cout << "Product: " << plan.product_name << " - " << plan.quantity
            << " " << plan.unit << (plan.quantity > 1 ? "s" : "") << ":" << endl;
        cout << "\tFixed capital: " << plan.fixed_capital << endl;
        cout << "\tRaw materials: " << plan.raw_materials << endl;
        cout << "\tLabor: " << plan.labor << endl;
    }
}

void Company::employ(Worker * worker) {
    workers.push_back(worker);
}
