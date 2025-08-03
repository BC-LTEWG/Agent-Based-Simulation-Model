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
#include "randomizer.hpp"

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
        new Society(Config{.plan_cycle_duration = 90, .fic = .8, .workday_length = 8.0});

    Good * good = new Good{.name = "Apples", .value = 4, .target_surplus = 0};
    society->add_good(good);

    Firm * firm = new Firm(society);
    society->add_firm(firm);

    Distributor * distributor = new Distributor(society);
    society->distributors.push_back(distributor);
    society->add_firm(distributor);

    const int population_size = 100;

    firm->add_project(new Project(society,
        Plan::from(good, society->config.plan_cycle_duration * population_size)));

    for (int i = 0; i < population_size; i++) {
        Worker * worker = new Worker(distributor,
            {randomizer::need_cycle({good},
                1.0,
                {
                    randomizer::DistributionType::UNIFORM,
                    1,
                    0,
                },
                {
                    randomizer::DistributionType::NORMAL,
                    1,
                    0,
                })},
            1e4);
        society->add_worker(worker);
        firm->employ(worker);
    }

    std::vector<double> x;
    std::vector<double> y1;
    std::vector<double> y2;
    std::vector<double> y3;
    std::vector<double> y4;
    std::vector<double> y5;
    std::vector<double> y6;
    std::vector<double> y7;

    std::vector<double> tick_times;

    for (int i = 0; i < 1e4; i++) {
        printf("\n ------- Tick cycle %d -------\n", i + 1);

        // for (auto & worker : society->workers) {
        //     worker->change_needs({randomizer::need_cycle({good},
        //         1.0,
        //         {
        //             randomizer::DistributionType::UNIFORM,
        //             1,
        //             0,
        //         },
        //         {
        //             randomizer::DistributionType::NORMAL,
        //             2 - static_cast<double>(i) / 10000,
        //             .5,
        //         })});
        // }

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

        double median_worker_wealth = 0;
        std::vector<double> worker_wealths;
        worker_wealths.reserve(society->workers.size());
        for (auto & worker : society->workers) {
            worker_wealths.push_back(worker->wealth());
        }
        std::sort(worker_wealths.begin(), worker_wealths.end());
        if (worker_wealths.size() % 2 == 0) {
            median_worker_wealth = (worker_wealths[worker_wealths.size() / 2 - 1] +
                                       worker_wealths[worker_wealths.size() / 2]) /
                                   2;
        } else {
            median_worker_wealth = worker_wealths[worker_wealths.size() / 2];
        }

        y5.push_back(median_worker_wealth);
        y7.push_back(worker_wealths.back());

        y6.push_back(society->reserve);

        printf("Median worker wealth: %.2f\n", median_worker_wealth);

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
    plt::plot(x, y5, {{"label", "Median Worker Wealth"}, {"color", "purple"}});
    plt::plot(x, y6, {{"label", "Society Reserve"}, {"color", "brown"}});
    // plt::plot(x, y7, {{"label", "Max Worker Wealth"}, {"color", "pink"}});
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
