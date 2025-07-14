#include "../include/Incentive.h"
#include "../include/IncentiveType.h"

Incentive::Incentive(Worker* w, Project* p) 
    : worker(w), project(p), incentiveType(nullptr), 
      incentivePriorityScoreForScarceGoods(0), incentiveHours(0) {
    
    if (worker && project) {
        calculateIncentiveHours();
    }
}

int Incentive::getIncentivePriorityScoreForScarceGoods() const {
    return incentivePriorityScoreForScarceGoods;
}

int Incentive::getIncentiveHours() const {
    return incentiveHours;
}

int Incentive::getPriorityQueuePosition() const {
    // Find position in scarce goods queue
    for (size_t i = 0; i < scarceGoodsQueue.size(); ++i) {
        if (scarceGoodsQueue[i].workerId == worker->getId()) {
            return static_cast<int>(i);
        }
    }
    return -1; // Not found in queue
}

void Incentive::calculateIncentiveHours() {
    if (!worker || !project) return;
    
    int workerHours = worker->getLaborTimeHoursWorked();
    int projectHours = project->getLaborTimeHoursPerWorker();
    
    if (workerHours < projectHours) {
        incentiveHours = projectHours - workerHours;
        
        // Apply incentive type adjustments
        if (incentiveType) {
            switch (*incentiveType) {
                case IncentiveType::REST:
                    incentiveHours = 0;
                    break;
                case IncentiveType::WORK_ON_DIFFERENT_PROJECT:
                    incentiveHours = 0;
                    break;
                case IncentiveType::MENTOR:
                    incentiveHours = 0;
                    break;
                default:
                    // Keep original incentive hours
                    break;
            }
        }
    } else {
        incentiveHours = 0;
    }
    
    // Calculate priority score based on incentive hours and worker experience
    incentivePriorityScoreForScarceGoods = incentiveHours + 
        (worker->getLaborTimeHoursWorked() / 100); // Experience factor
}
