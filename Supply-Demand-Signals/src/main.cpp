#include <cassert>
#include <chrono>
#include <cstdio>
#include <vector>

#include "Distributor.hpp"
#include "Firm.hpp"
#include "Plan.hpp"
#include "PlotHandler.hpp"
#include "Society.hpp"
#include "Worker.hpp"
#include "randomizer.hpp"

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

    Good * means = new Good("Flour", 5e3, {}, 1);
    Good * good = new Good("Bread", 5e3, {{means, 1}}, 1);
    society->add_good(means);
    society->add_good(good);

    Firm * means_firm = new Firm(society);
    Firm * good_firm = new Firm(society);
    society->add_firm(means_firm);
    society->add_firm(good_firm);

    Distributor * distributor = new Distributor(society);
    society->distributors.push_back(distributor);
    society->add_firm(distributor);

    const int population_size = 100;

    auto good_project = good_firm->add_project(new Project(society,
        Plan::from(good,
            society->config.plan_cycle_duration * population_size + good->target_surplus)));

    means_firm->add_project(new Project(society,
        Plan::from(means,
            good_project->plan.quantity * good->means[means] + means->target_surplus)));

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
        society->distribute_worker(worker, i, population_size);
    }

    auto plot =
        PlotHandler(1200, 900, "Economic statistics over time", "Plan cycle count", "Quantity");
    plot.define("Bread Inventory", {0.1f, 0.1f, 0.8f});                   // Dark Blue
    plot.define("Flour Inventory", {0.1f, 0.8f, 0.8f})->hide();           // Cyan
    plot.define("Bread Production Deficit", {0.8f, 0.1f, 0.1f});          // Dark Red
    plot.define("Flour Production Deficit", {0.8f, 0.1f, 0.8f})->hide();  // Magenta
    plot.define("Bread Planned Production", {0.1f, 0.8f, 0.1f});          // Green
    plot.define("Flour Planned Production", {0.8f, 0.8f, 0.1f})->hide();  // Yellow
    plot.define("Bread Actual Production", {0.2f, 0.2f, 0.2f});           // Dark Gray
    plot.define("Flour Actual Production", {0.8f, 0.8f, 0.8f})->hide();   // Light Gray
    plot.define("Bread Ideal Workers", {0.1f, 0.1f, 0.8f})->hide();       // Dark Blue
    plot.define("Flour Ideal Workers", {0.1f, 0.8f, 0.8f})->hide();       // Cyan
    plot.define("Median Worker Wealth", {0.8f, 0.1f, 0.8f})->hide();      // Magenta
    plot.define("Society Reserve", {0.2f, 0.2f, 0.2f})->hide();           // Dark Gray

    std::vector<double> tick_times;

    for (int i = 0; i < 1e3; i++) {
        printf("\n ------- Tick cycle %d -------\n", i + 1);

        auto start_time = std::chrono::high_resolution_clock::now();
        society->tick_cycle(i == 0);
        auto end_time = std::chrono::high_resolution_clock::now();

        auto duration =
            std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        tick_times.push_back(duration.count() / 1000.0);

        distributor->head();

        plot.add_x(i);
        plot.add("Bread Inventory", distributor->total_inventory(good));
        plot.add("Flour Inventory", distributor->total_inventory(means));
        plot.add("Bread Production Deficit", distributor->get_production_deficit(good, i));
        plot.add("Flour Production Deficit", distributor->get_production_deficit(means, i));
        plot.add("Bread Planned Production", good_firm->all_projects()[0]->plan.quantity);
        plot.add("Flour Planned Production", means_firm->all_projects()[0]->plan.quantity);
        plot.add("Bread Actual Production", good_firm->all_projects()[0]->goods_produced);
        plot.add("Flour Actual Production", means_firm->all_projects()[0]->goods_produced);
        plot.add("Bread Ideal Workers", good_firm->all_projects()[0]->ideal_workers * 1000);
        plot.add("Flour Ideal Workers", means_firm->all_projects()[0]->ideal_workers * 1000);

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

        plot.add("Median Worker Wealth", median_worker_wealth);

        plot.add("Society Reserve", society->reserve);

        printf("Median worker wealth: %.2f\n", median_worker_wealth);

        double avg_worker_needs = 0;
        for (auto & worker : society->workers) {
            avg_worker_needs += worker->need_count();
        }
        avg_worker_needs /= society->workers.size();
        printf("Average worker needs: %.2f\n", avg_worker_needs);
        // plot.add("Average Worker Needs", avg_worker_needs * 1e5);
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

    plot.plot();

    if (arg_present("--save")) {
        plot.save("img/output.png");
    }
    if (arg_present("--show")) {
        plot.show();
    }

    return 0;
}
