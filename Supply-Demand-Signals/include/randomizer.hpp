#pragma once

#include <vector>

#include "Worker.hpp"
namespace randomizer {
    enum DistributionType { UNIFORM, NORMAL };

    struct Distribution {
            DistributionType type;
            double mean;
            double stddev;
    };

    WorkerNeedCycle need_cycle(std::vector<Good *> good_options, double priority,
        Distribution cycles, Distribution amount);
}  // namespace randomizer