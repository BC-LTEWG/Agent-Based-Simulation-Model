#include "../include/Worker.hpp"

#include <algorithm>
#include <cstdio>

Worker::Worker(Distributor * distributor, std::vector<WorkerNeedCycle> need_cycles)
    : distributor(distributor), need_cycles(need_cycles) {
    for (auto & need_cycle : need_cycles) {
        need_cycle.clock = need_cycle.cycles;
    }
}

void Worker::pay(double amount) { credits += amount; }

bool Worker::spend(double amount) {
    if (credits >= amount) {
        credits -= amount;
        return true;
    }
    return false;
}

void Worker::update_needs() {
    for (auto & need_cycle : need_cycles) {
        need_cycle.clock--;
        if (need_cycle.clock <= 0) {
            // Time to replenish this need
            needs.push_back(new WorkerNeed{*need_cycle.need});
            // Reset the clock
            need_cycle.clock = need_cycle.cycles;
        }
    }

    // Sort needs by priority (lower number means higher priority)
    std::sort(needs.begin(), needs.end(), [](WorkerNeed * a, WorkerNeed * b) {
        return a->priority < b->priority;
    });
}

void Worker::fulfill_needs() {
    int need_ptr = 0;
    while (need_ptr < (int)needs.size()) {
        auto need = needs[need_ptr];
        double purchaseable_amount = credits / need->good->value;
        double amount_to_buy = std::min(purchaseable_amount, need->amount);
        if (amount_to_buy <= 0) {
            break;
        }
        auto [purchased_amount, cost] = distributor->purchase(need->good, amount_to_buy);

        if (!spend(cost)) {
            break;  // Not enough credits
        }
        need->amount -= purchased_amount;
        if (need->amount <= 0) {
            needs.erase(needs.begin() + need_ptr);
            delete need;
        } else {
            need_ptr++;
        }
    }
}