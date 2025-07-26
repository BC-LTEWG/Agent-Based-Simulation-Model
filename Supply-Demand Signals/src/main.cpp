#include <cassert>
#include <cstdio>
#include <vector>

#include "../extern/matplotlib-cpp/matplotlibcpp.h"
#include "../include/Distributor.hpp"
#include "../include/Firm.hpp"
#include "../include/Plan.hpp"
#include "../include/Society.hpp"
#include "../include/Worker.hpp"

namespace plt = matplotlibcpp;

int main() {
    // Quarterly plan cycle duration
    Society * society =
        new Society(Config{.plan_cycle_duration = 90, .fic = 0.8, .workday_length = 8.0});

    Good * good = new Good{.name = "Apples", .value = .10};
    society->add_good(good);

    Firm * firm = new Firm(society);
    society->add_firm(firm);

    Distributor * distributor = new Distributor(society);
    society->distributors.push_back(distributor);
    society->add_firm(distributor);

    firm->add_project(new Project(society,
        Plan{.means = 0, .resources = 0, .labor = 90 * 8 * 10, .good = good, .quantity = 1000}));

    const int population_size = firm->total_ideal_jobs();
    for (int i = 0; i < population_size; i++) {
        Worker * worker = new Worker(distributor,
            {{.need = new WorkerNeed{.good = good, .amount = 1, .priority = 1}, .cycles = 1}});
        society->add_worker(worker);
        assert(firm->employ(worker));
    }

    std::vector<double> x;
    std::vector<double> y;
    std::vector<double> y2;

    for (int i = 0; i < 10; i++) {
        printf("\n ------- Tick cycle %d -------\n", i);
        society->tick_cycle(i == 0);
        distributor->head();

        x.push_back(i);
        y.push_back(distributor->total_inventory(good));
        y2.push_back(distributor->get_production_deficit(good, i));
    }

    plt::figure_size(800, 600);
    plt::plot(x, y, "b-");
    plt::plot(x, y2, "r-");
    plt::title("Inventory and Production Deficit of Apples Over Time");
    plt::xlabel("Time (cycles)");
    plt::ylabel("Quantity");
    plt::show();

    return 0;
}
