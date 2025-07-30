#include <cassert>
#include <chrono>
#include <cstdio>
#include <map>
#include <vector>

#include "../extern/matplotlib-cpp/matplotlibcpp.h"
#include "../include/Distributor.hpp"
#include "../include/Firm.hpp"
#include "../include/Plan.hpp"
#include "../include/Society.hpp"
#include "../include/Worker.hpp"

namespace plt = matplotlibcpp;

int main(int argc, char * argv[]) {
    auto arg_present = [&argc, &argv](const char * arg) {
        for (int i = 0; i < argc; i++) {
            if (strcmp(argv[i], arg) == 0) {
                return true;
            }
        }
        return false;
    };

    // Quarterly plan cycle duration
    Society * society =
        new Society(Config{.plan_cycle_duration = 90, .fic = 0.8, .workday_length = 8.0});

    Good * good = new Good{.name = "Apples", .value = .20};
    society->add_good(good);

    Firm * firm = new Firm(society);
    society->add_firm(firm);

    Distributor * distributor = new Distributor(society);
    society->distributors.push_back(distributor);
    society->add_firm(distributor);

    firm->add_project(new Project(society,
        Plan{.means = 0, .resources = 0, .labor = 90 * 8 * 10, .good = good, .quantity = 500}));

    const int population_size = firm->total_ideal_jobs();
    for (int i = 0; i < population_size; i++) {
        Worker * worker = new Worker(distributor,
            {{.need = new WorkerNeed{.good = good, .amount = 1, .priority = 1}, .cycles = 1}});
        society->add_worker(worker);
        assert(firm->employ(worker));
    }

    std::vector<double> x;
    std::vector<double> y1;
    std::vector<double> y2;
    std::vector<double> y3;
    std::vector<double> y4;

    std::vector<double> tick_times;

    for (int i = 0; i < 300; i++) {
        printf("\n ------- Tick cycle %d -------\n", i + 1);

        auto start_time = std::chrono::high_resolution_clock::now();
        society->tick_cycle(i == 0);
        auto end_time = std::chrono::high_resolution_clock::now();

        auto duration =
            std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        tick_times.push_back(duration.count() / 1000.0);

        distributor->head();

        x.push_back(i);
        y1.push_back(distributor->total_inventory(good));
        y2.push_back(distributor->get_production_deficit(good, i));
        y3.push_back(firm->all_projects()[0]->plan.quantity);
        y4.push_back(firm->all_projects()[0]->goods_produced);

        double avg_worker_needs = 0;
        for (auto & worker : society->workers) {
            avg_worker_needs += worker->need_count();
        }
        avg_worker_needs /= society->workers.size();
        printf("Average worker needs: %.2f\n", avg_worker_needs);
    }

    double total_tick_time = 0.0;
    for (double time : tick_times) {
        total_tick_time += time;
    }
    double average_tick_time = total_tick_time / tick_times.size();

    printf("\n------- Performance Summary -------\n");
    printf("Total ticks: %zu\n", tick_times.size());
    printf("Total tick time: %.2f ms\n", total_tick_time);
    printf("Average tick time: %.2f ms\n", average_tick_time);

    if (!arg_present("--save") && !arg_present("--show")) {
        return 0;
    }

    plt::figure_size(800, 600);
    plt::plot(x, y1, {{"label", "Inventory"}, {"color", "blue"}});
    plt::plot(x, y2, {{"label", "Production Deficit"}, {"color", "red"}});
    plt::plot(x, y3, {{"label", "Planned Production"}, {"color", "green"}});
    plt::plot(x, y4, {{"label", "Actual Production"}, {"color", "orange"}});
    plt::title("Inventory and Production Deficit of Apples Over Time");
    plt::xlabel("Time (cycles)");
    plt::ylabel("Quantity");
    plt::legend();

    plt::grid(true);

    for (int i = 0; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");

    if (arg_present("--save")) {
        plt::save("img/output.png");
    } else if (arg_present("--show")) {
        plt::show();
    }

    return 0;
}
