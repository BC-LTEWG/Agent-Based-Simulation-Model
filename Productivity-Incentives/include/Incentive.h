#pragma once
#include "IncentiveType.h"
#include "Worker.h"
#include "Project.h"
#include <vector>

// Forward declaration for priority queue
struct PriorityQueueForScarceGoods {
    int workerId;
    int priority;
    std::vector<GoodsService> requestedGoods;
};

class Incentive {
    public:
        Incentive(Worker * w, Project * p);

        // Getters
        int getIncentivePriorityScoreForScarceGoods() const;
        int getIncentiveHours() const;
        int getPriorityQueuePosition() const;
        
        // Public members (as originally designed)
        Worker * worker;
        Project * project;
        IncentiveType * incentiveType;
        int incentivePriorityScoreForScarceGoods;
        std::vector<PriorityQueueForScarceGoods> scarceGoodsQueue;
        int incentiveHours;

    private:
        void calculateIncentiveHours();
};