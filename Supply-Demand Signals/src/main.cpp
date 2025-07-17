#include <cassert>
#include <cstdio>

#include "../include/Distributor.hpp"
#include "../include/Firm.hpp"
#include "../include/Plan.hpp"
#include "../include/Society.hpp"
#include "../include/Worker.hpp"

int main() {
    // Quarterly plan cycle duration
    Society * society = new Society(90);

    Good * good = new Good{.name = "Apples", .value = .10};
    society->add_good(good);

    Firm * firm = new Firm(society);
    society->add_firm(firm);

    Distributor * distributor = new Distributor(society);
    society->distributors.push_back(distributor);

    firm->add_project(new Project(society,
        Plan{.means = 0, .resources = 0, .labor = 90 * 8 * 10, .good = good, .quantity = 1000}));

    const int population_size = firm->total_ideal_jobs();
    for (int i = 0; i < population_size; i++) {
        Worker * worker = new Worker(distributor,
            {{.need = new WorkerNeed{.good = good, .amount = 1, .priority = 1}, .cycles = 1}});
        society->add_worker(worker);
        assert(firm->employ(worker));
    }

    for (int i = 0; i < 10; i++) {
        printf("Tick cycle %d\n", i);
        society->tick_cycle();
        distributor->head();
    }

    return 0;
}
