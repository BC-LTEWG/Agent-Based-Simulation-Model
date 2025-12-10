#include "../include/Worker.hpp"

Worker::Worker(Distributor * distributor, std::vector<WorkerNeedCycle> need_cycles,
    double initial_credits)
    : distributor(distributor), credits(initial_credits), need_cycles(need_cycles) {
    // filter out zeroed cycles
    need_cycles.erase(std::remove_if(need_cycles.begin(),
                          need_cycles.end(),
                          [](const WorkerNeedCycle & cycle) { return cycle.need->amount <= 0; }),
        need_cycles.end());

    for (auto & need_cycle : need_cycles) {
        need_cycle.clock = need_cycle.cycles;
    }
}

double Worker::wealth() { return credits; }

void Worker::pay(double amount) { credits += amount; }

bool Worker::spend(double amount) {
    if (credits >= amount) {
        credits -= amount;
        return true;
    }
    return false;
}

int Worker::need_count() { return needs.size(); }

std::vector<WorkerNeed *> Worker::cycle_needs() {
    std::vector<WorkerNeed *> new_needs;
    new_needs.reserve(need_cycles.size());

    robin_hood::unordered_set<Good *> needed_goods;
    needed_goods.reserve(needs.size());

    for (auto * need : needs) {
        needed_goods.insert(need->good);
    }

    for (auto & need_cycle : need_cycles) {
        need_cycle.clock--;
        if (need_cycle.clock <= 0) {
            // Time to replenish this need, only if we don't already need it
            if (!needed_goods.count(need_cycle.need->good)) {
                new_needs.push_back(new WorkerNeed{*need_cycle.need});
            }
            // Reset the clock
            need_cycle.clock = need_cycle.cycles;
        }
    }

    // Sort new needs by priority and merge into existing sorted list
    std::sort(new_needs.begin(), new_needs.end(), [](WorkerNeed * a, WorkerNeed * b) {
        return a->priority < b->priority;
    });

    return new_needs;
}

void Worker::update_needs() {
    auto new_needs = cycle_needs();

    std::vector<WorkerNeed *> merged;

    merged.reserve(needs.size() + new_needs.size());

    std::merge(new_needs.begin(),
        new_needs.end(),
        needs.begin(),
        needs.end(),
        std::back_inserter(merged),
        [](WorkerNeed * a, WorkerNeed * b) { return a->priority < b->priority; });

    needs.swap(merged);
}

void Worker::fulfill_needs() {
    std::vector<WorkerNeed *> kept;
    kept.reserve(needs.size());
    for (size_t i = 0; i < needs.size(); ++i) {
        WorkerNeed * need = needs[i];
        const double purchaseable_amount = credits / need->good->value;
        // double purchaseable_amount = need->amount;
        const double amount_to_buy = std::min(purchaseable_amount, need->amount);

        // const double amount_cant_buy = need->amount - amount_to_buy;
        // distributor->simulate_need(need->good, amount_cant_buy);

        if (amount_to_buy <= 0) {
            // cannot buy more, preserve remaining needs and exit
            kept.insert(kept.end(), needs.begin() + i, needs.end());
            break;
        }
        auto [purchased_amount, cost] = distributor->purchase(need->good, amount_to_buy);
        if (!spend(cost)) {
            kept.insert(kept.end(), needs.begin() + i, needs.end());
            break;  // Not enough credits
        }
        need->amount -= purchased_amount;
        if (need->amount > 0) {
            kept.push_back(need);
        } else {
            delete need;
        }
    }
    needs.swap(kept);
}