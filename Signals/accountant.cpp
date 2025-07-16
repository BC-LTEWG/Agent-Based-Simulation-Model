#include "accountant.h"
#include "company.h"

bool Accountant::approve_plan(Plan * draft_plan, Inputs * amended_inputs) {
    Company * company = draft_plan->company;
    int num_plans = company->plans.size();
    if (!num_plans) {
       return true;
    }
    Plan * last_plan = company->plans.back();
    if (!last_plan->product) {
        return true;
    }
    amended_inputs->labor += last_plan->product;
    return false;
}

