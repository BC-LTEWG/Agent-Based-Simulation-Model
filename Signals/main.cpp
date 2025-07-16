#include <iostream>
#include <cstdlib>

#include "accountant.h"
#include "company.h"

void add_plan_to_company(Company * company, Accountant * accountant) {
    Plan * plan = new Plan();
    plan->product_name = "widget";
    plan->quantity = 16;
    plan->unit = "item";
    plan->inputs.raw_materials = 10.0;
    plan->inputs.labor = 20.0;
    Inputs amended_inputs;
    bool approved = accountant->approve_plan(plan, &amended_inputs);
    if (!approved) {
        plan->inputs = amended_inputs;
    }
    company->add_plan(plan);
}

int main() {
    Company * company = new Company("A");
    Accountant  accountant;
    cout << "Company: " << company->name() << endl;
    add_plan_to_company(company, &accountant);
    company->print_plans();
    return EXIT_SUCCESS;
}

