#pragma once

#include <unordered_map>
#include <cmath>

#include "Firm.h"
#include "Machine.h"
#include "Person.h"

class Firm;
class Person;

const double PRODUCTIVITY_THRESHOLD = 60.0;

class Society {
  public:
    std::vector<Person*> workers, availableWorkers;
    std::vector<Firm*> firms;
    std::unordered_map<Firm*, double> prices;
	static double randomRetirementChance, averageRetirementAge;
	static int guaranteedRetirementAge;
    const int initial_work_hours_daily = 8;
    int current_work_hours_daily = initial_work_hours_daily;

    Society(std::vector<Person*> workers, std::vector<Firm*> firms, std::unordered_map<Firm*, double> prices);

    std::size_t num_workers();
    std::size_t num_firms();
    bool meets_standard_for_lower_working_hours();
    void set_work_hours_daily(int hours);
    double get_avg_productivity();
    std::unordered_map<std::string, int> avg_worker_needs();
	Person& birthWorker();
	void birthWorkersEveryTimestep();
	void retireWorker(Person& worker);
};
