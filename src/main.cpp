#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <unordered_map>
#include <unordered_set>

#include "Constants.h"
#include "Sim.h"

void print_usage() {
    std::cout << "Usage: sim [<args> ...]" << std::endl;
    std::cout << "\t-n N: Run the simulation for N time steps." << std::endl;
    std::cout << "\t-p N: Simulate a society with N people." << std::endl;
    std::cout << "\t-h N: Set the workday to N hours." << std::endl;
    std::cout << "\t-w N: Set the work week to N days." << std::endl;
    std::cout << "\t-g N: Set the initial number of goods to N." << std::endl;
    std::cout << "\t-m N: Set the initial number of machines to N." <<
        std::endl;
    std::cout << "\t-i N: Set the maximum number of inputs per product to N."
        << std::endl;
    std::cout << "\t-r N: Set the initial number of producers to N." <<
        std::endl;
    std::cout << "\t-d N: Set the initial number of distributors to N." <<
        std::endl;
    std::cout << "\t-a N: Set the number of modeled abilities (work skills) "
        "to N." << std::endl;
    std::cout << "\t-v N: Set the standard deviation of abilities to N." <<
        std::endl;
    std::cout << "\t-e N: Set the random seed to N." << std::endl;
    std::cout << "\t-s N: Set the annual chance of an agent getting sick." <<
        std::endl;
    std::cout << "\t--init_prices S: How initial prices are set. Options are "
        "'labor_values' and 'equilibrium_prices'." << std::endl;
    std::cout << "\t--production_difficulty N: Set the spectral radius of "
        "the I/O requirements matrix. Higher values make surplus generation "
        "more difficult." << std::endl;
    std::cout << "\t--consumption_difficulty N: Set the proportion of value "
        "consumed by society relative to the maximum value producable by "
        "that society." << std::endl;
    std::cout << "\t--public_sector_expansion_period N: Period (in months) "
        "of moving goods to be funded by the public sector, transitioning to "
        "communism. N = 0 results in no public sector expansion." << std::endl;
    std::cout << "\t-j: Write JSON log traces to stdout." << std::endl;
    std::cout << "\t-u: Print this usage message." << std::endl;
}

enum class ArgType {
    kTimeSteps,
    kPeople,
    kWorkHours,
    kWorkDays,
    kGoods,
    kMachines,
    kMaxInputs,
    kProducers,
    kDistributors,
    kAbilities,
    kAbilityStdDev,
    kSickChance,
    kSeed,
    kProductionDifficulty,
    kConsumptionDifficulty,
    kPublicSectorExpansionPeriod,
    kInitPrices
};

static const std::unordered_set<std::string> valid_init_price_modes = {
    "labor_values",
    "equilibrium_prices"
};

void set_params(int argc, const char ** argv, SimArgs& args) {
    bool error = false;

    static const std::unordered_map<std::string, ArgType> valid_args = {
        {"-n", ArgType::kTimeSteps}, {"--time-steps", ArgType::kTimeSteps},
        {"-p", ArgType::kPeople}, {"--people", ArgType::kPeople},
        {"-h", ArgType::kWorkHours}, {"--work-hours", ArgType::kWorkHours},
        {"-w", ArgType::kWorkDays}, {"--work-days", ArgType::kWorkDays},
        {"-g", ArgType::kGoods}, {"--goods", ArgType::kGoods},
        {"-m", ArgType::kMachines}, {"--machines",ArgType::kMachines},
        {"-i", ArgType::kMaxInputs}, {"--products-per-machine",
                                            ArgType::kMaxInputs},
        {"-r", ArgType::kProducers}, {"--producers", ArgType::kProducers},
        {"-d", ArgType::kDistributors}, {"--distributors",
                                            ArgType::kDistributors},
        {"-a", ArgType::kAbilities}, {"--abilities", ArgType::kAbilities},
        {"-v", ArgType::kAbilityStdDev}, {"--ability-stddev",
                                             ArgType::kAbilityStdDev},
        {"-s", ArgType::kSickChance}, {"--sick-chance", ArgType::kSickChance},
        {"-e", ArgType::kSeed}, {"--seed", ArgType::kSeed},
        {"--production_difficulty", ArgType::kProductionDifficulty},
        {"--consumption_difficulty", ArgType::kConsumptionDifficulty},
        {"--public_sector_expansion_period",
            ArgType::kPublicSectorExpansionPeriod},
        {"--init_prices", ArgType::kInitPrices}
    };


    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-u" || arg == "--usage") {
            print_usage();
            exit(EXIT_SUCCESS);
        }
        if (arg == "-j" || arg == "--json") { //"j" needs no additional value
            args.json = true;
            continue;
        }
        if (i + 1 >= argc) {
            error = true;
            break;
        }
        if (!valid_args.count(arg)) {
            error = true;
            break;
        }
        long value = strtol(argv[++i], nullptr, 10);
        double dvalue = strtod(argv[i], nullptr);
        switch (valid_args.at(arg)) {
            case ArgType::kTimeSteps: {
                if (value <= 0) {
                    error = true;
                } else {
                    args.time_steps = value;
                }
                break;
            }
            case ArgType::kPeople: {
                if (value <= 0) {
                    error = true;
                } else {
                    args.num_people = value;
                }
                break;
            }
            case ArgType::kWorkHours: {
                if (value <= 0 || value > 24) {
                    error = true;
                } else {
                    args.work_hours_daily = value;
                }
                break;
            }
            case ArgType::kWorkDays: {
                if (value <= 0 || value > 7) {
                    error = true;
                } else {
                    args.work_days_weekly = value;
                }
                break;
            }
            case ArgType::kGoods: {
                if (value <= 0) {
                    error = true;
                } else {
                    args.num_goods = value;
                }
                break;
            }
            case ArgType::kMachines: {
                if (value < 0) {
                    error = true;
                } else {
                    args.num_machines = value;
                }
                break;
            }
            case ArgType::kMaxInputs: {
                if (value < 0) {
                    error = true;
                } else {
                    args.max_num_inputs = value;
                }
                break;
            }
            case ArgType::kProducers: {
                if (value <= 0) {
                    error = true;
                } else {
                    args.num_producers = value;
                }
                break;
            }
            case ArgType::kDistributors: {
                if (value <= 0) {
                    error = true;
                } else {
                    args.num_distributors = value;
                }
                break;
            }
            case ArgType::kAbilities: {
                if (value <= 0) {
                    error = true;
                } else {
                    args.num_abilities = value;
                }
                break;
            }
            case ArgType::kAbilityStdDev: {
                if (dvalue < 0.0) {
                    error = true;
                } else {
                    args.ability_stddev = dvalue;
                }
                break;
            }
            case ArgType::kSickChance: {
                if (dvalue < 0.0 || dvalue > 1.0) {
                    error = true;
                } else {
                    args.sickness_chance = dvalue;
                }
                break;
            }
            case ArgType::kProductionDifficulty: {
                if (dvalue < 0.0 || dvalue >= 1.0) {
                    error = true;
                } else {
                    args.production_difficulty = dvalue;
                }
                break;
            }
            case ArgType::kConsumptionDifficulty: {
                if (dvalue < 0.0 || dvalue >= 1.0) {
                    error = true;
                } else {
                    args.consumption_difficulties[0] = dvalue;
                }
                std::ifstream file("../runtime/consumption_difficulties.in");
                if (file.is_open()) {
                    while (!file.eof()) {
                        int time_step; 
                        file >> time_step;
                        double difficulty;
                        file >> difficulty;
                        if (time_step < 0 || difficulty < 0.0 || difficulty >= 1.0) {
                            error = true;
                            break;
                        }
                        args.consumption_difficulties[time_step] = difficulty;
                    }
                }
                break;
            }
            case ArgType::kPublicSectorExpansionPeriod: {
                if (value < 0) {
                    error = true;
                } else {
                    args.public_sector_expansion_period = value * MONTH;
                }
                break;
            }
            case ArgType::kSeed: {
                if (value < 0) {
                    error = true;
                } else {
                    args.seed = value;
                    args.fixed_seed = true;
                }
                break;
            }
            case ArgType::kInitPrices: {
                if (valid_init_price_modes.count(argv[i])) {
                    args.init_price_mode = argv[i];

                } else {
                    std::cerr << "Warning! Unknown initial price mode " <<
                        argv[i] << ". Defaulting to labor_values" <<
                        std::endl;
                    args.init_price_mode = "labor_values";
                }
                break;
            }
        }
    }
    if (error) {
        print_usage();
        exit(EXIT_FAILURE);
    }
}

int main(int argc, const char ** argv) {
    SimArgs args;
    set_params(argc, argv, args);
    Sim::run(args);
	return EXIT_SUCCESS;
}
