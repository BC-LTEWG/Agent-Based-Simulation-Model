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

    auto plot_handler = PlotHandler(1200, 900);
    auto main_plot = plot_handler.create_plot("main",
        "Economic statistics over time",
        "Plan cycle count",
        "Quantity");
    main_plot->define_line("Median Worker Wealth", {0.8f, 0.1f, 0.8f});  // Magenta
    main_plot->define_line("Society Reserve", {0.2f, 0.2f, 0.2f});       // Dark Gray

    robin_hood::unordered_map<Good *, Plot *> good_plots;
    for (auto & good : society->goods) {
        good_plots[good] = plot_handler.create_plot(good->name,
            good->name + " over time",
            "Plan cycle count",
            "Quantity");

        // Define the lines for this good's plot
        good_plots[good]->define_line("Inventory", {0.2f, 0.6f, 0.9f});           // Blue
        good_plots[good]->define_line("Production Deficit", {0.9f, 0.2f, 0.2f});  // Red
        good_plots[good]->define_line("Planned Production", {0.2f, 0.9f, 0.2f});  // Green
        good_plots[good]->define_line("Actual Production", {0.9f, 0.7f, 0.2f});   // Orange
        good_plots[good]->define_line("Ideal Workers", {0.6f, 0.2f, 0.9f});       // Purple
    }

    main_plot->hide();
    good_plots[good]->hide();

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

        main_plot->add_x(i);

        for (auto & [good, plot] : good_plots) {
            plot->add_x(i);
            plot->add_y("Inventory", distributor->total_inventory(good));
            plot->add_y("Production Deficit", distributor->get_production_deficit(good, i));

            auto stats = society->aggregate_good_stats(good);

            plot->add_y("Planned Production", stats.planned_quantity);
            plot->add_y("Actual Production", stats.produced);
            plot->add_y("Ideal Workers", stats.ideal_workers * 1000);
        }

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

        main_plot->add_y("Median Worker Wealth", median_worker_wealth);

        main_plot->add_y("Society Reserve", society->reserve);

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

    plot_handler.plot();

    if (arg_present("--save")) {
        plot_handler.save("img/output.png");
    }
    if (arg_present("--show")) {
        plot_handler.show();
    }

    return 0;
}
