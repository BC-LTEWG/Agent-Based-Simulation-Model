#pragma once 
#include "Firm.h"
#include "Worker.h"
#include "Machine.h"
#include <unordered_map>

const double productivityThreshold = 60.0; // threshold for productivity before lowering working hours

class Society{
  public:
    std::vector<Worker&> workers;
    std::vector<Firm&> firms;
    std::unordered_map<Firm&, double> prices;
    const int initial_work_hours_daily = 8;
    int current_work_hours_daily = initial_work_hours_daily;

    Society(std::vector<Worker&> Workers, std::vector<Firm&> firms, std::unordered_map<Firm&, double> prices) : workers(Workers), firms(firms), prices(prices) {}
    
    std::size_t numWorkers() {return workers.size()};
    std::size_t numFirms() {return firms.size()};
    bool meetsStandardForLowerWorkingHours() {};
    void setWorkHoursDaily(int hours) {current_work_hours_daily = hours;};
    



};
