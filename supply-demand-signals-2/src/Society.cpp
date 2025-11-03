#include <numeric>
#include <cmath>

#include "Society.h"

double Society::randomRetirementChance = 8.3e-7;

// approx 50 years in hours
int Society::guaranteedRetirementAge = 438300;

// should be approx 45 years in hours
double Society::averageRetirementAge = (1 - pow(1 - Society::randomRetirementChance, Society::guaranteedRetirementAge)) / Society::randomRetirementChance;

Society::Society(std::vector<Person*> workers, std::vector<Firm*> firms, std::unordered_map<Firm*, double> prices) 
    : workers(workers), firms(firms), prices(prices) {}

std::size_t Society::num_workers() {
    return workers.size();
}

std::size_t Society::num_firms() {
    return firms.size();
}

bool Society::meets_standard_for_lower_working_hours() {
    double sum = 0.0;
    for(auto firm : firms) {
        sum += firm->get_avg_productivity();
    }
    double avgProductivity = sum / firms.size();
    return avgProductivity >= PRODUCTIVITY_THRESHOLD;
}

void Society::set_work_hours_daily(int hours) {
    current_work_hours_daily = hours;
}

double Society::get_avg_productivity() {
    return 0.0;
}

std::unordered_map<std::string, int> Society::avg_worker_needs() {
    return std::unordered_map<std::string, int>();
}
