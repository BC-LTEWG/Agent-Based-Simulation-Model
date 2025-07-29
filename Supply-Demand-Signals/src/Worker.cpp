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

int Worker::need_count() {
    return needs.size();
}

void Worker::update_needs() {
    std::vector<WorkerNeed *> new_needs;
    for (auto & need_cycle : need_cycles) {
        need_cycle.clock--;
        if (need_cycle.clock <= 0) {
            // Time to replenish this need
            new_needs.push_back(new WorkerNeed{*need_cycle.need});
            // Reset the clock
            need_cycle.clock = need_cycle.cycles;
        }
    }

    // Sort needs by priority (lower number means higher priority)
    std::sort(new_needs.begin(), new_needs.end(), [](WorkerNeed * a, WorkerNeed * b) {
        return a->priority < b->priority;
    });

    int need_ptr = 0;
    int new_need_ptr = 0;

    while (new_need_ptr < (int)new_needs.size() && need_ptr < (int)needs.size()) {
        if (new_needs[new_need_ptr]->priority < needs[need_ptr]->priority) {
            // Insert new need before current need
            needs.insert(needs.begin() + need_ptr, new_needs[new_need_ptr]);
            new_need_ptr++;
        } else {
            // Keep current need, move to next
            need_ptr++;
        }
    }

    while (new_need_ptr < (int)new_needs.size()) {
        needs.push_back(new_needs[new_need_ptr]);
        new_need_ptr++;
    }

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
        auto [purchased_amount, cost] = distributor->purchase(this, need->good, amount_to_buy);

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