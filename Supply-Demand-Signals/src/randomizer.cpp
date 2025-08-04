#include "../include/randomizer.hpp"

#include <algorithm>
#include <random>

#include "Worker.hpp"

namespace randomizer {
    namespace utils {
        template <typename T>
        T random_choice(const std::vector<T> & vec) {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_int_distribution<size_t> dis(0, vec.size() - 1);
            return vec[dis(gen)];
        }

        double use_distribution(Distribution & distribution) {
            static std::random_device rd;
            static std::mt19937 gen(rd());

            if (distribution.type == UNIFORM) {
                std::uniform_real_distribution<double> dis(distribution.mean - distribution.stddev,
                    distribution.mean + distribution.stddev);
                return dis(gen);
            } else if (distribution.type == NORMAL) {
                std::normal_distribution<double> dis(distribution.mean, distribution.stddev);
                return dis(gen);
            } else {
                throw std::invalid_argument("Unknown distribution type");
            }
        }
    }  // namespace utils

    WorkerNeedCycle need_cycle(std::vector<Good *> good_options, double priority,
        Distribution cycles, Distribution amount) {
        if (good_options.empty()) {
            throw std::invalid_argument("Good options cannot be empty");
        }

        auto good = utils::random_choice(good_options);
        int cycle_count =
            std::max(0, static_cast<int>(std::round(utils::use_distribution(cycles))));
        double amt = utils::use_distribution(amount);
        if (good->discrete) {
            amt = std::round(amt);
        }

        WorkerNeedCycle cycle = {
            .need =
                new WorkerNeed{
                    .good = good,
                    .amount = amt,
                    .priority = priority,
                },
            .clock = cycle_count,
        };

        return cycle;
    }

}  // namespace randomizer