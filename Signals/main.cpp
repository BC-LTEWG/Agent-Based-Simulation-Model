#include <iostream>
#include <cstdlib>

#include "company.h"

void add_plan_to_company(Company * company) {
    Plan * plan = new Plan();
    plan->product_name = "widget";
    plan->quantity = 16;
    plan->unit = "item";
    plan->inputs.raw_materials = 10.0;
    plan->inputs.labor = 20.0;
    company->add_plan(plan);
}

int main() {
    Company * company = new Company("A");
    cout << "Company: " << company->name() << endl;
    add_plan_to_company(company);
    company->print_plans();
    return EXIT_SUCCESS;
}

