#pragma once

struct Plan {
	// independent/input fields
	Order * order;
    Firm * firm;
	std::vector<Person *> workers;
    unsigned int local_work_hours_daily;

	// dependent/output fields	
	double predicted_turnaround_time;
    double machinery_cost;
    double labor_hours;
    double raw_materials;
    double total_hours;
    double prd;
    double labor_hours_remaining;
    double raw_materials_remaining;
    double total_hours_remaining;
    double quantity_remaining;
	int outgoing_units_consumed;
};


