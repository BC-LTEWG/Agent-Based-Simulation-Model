#include <iostream>
#include <string>
#include <cstdlib>
#include <unordered_map>
#include <unordered_set>

#include "Sim.h"

void print_usage() {
    std::cout << "Usage: sim [<args> ...]" << std::endl;
    std::cout << "\t-n N: Run the simulation for N time steps." << std::endl;
    std::cout << "\t-p N: Simulate a society with N people." << std::endl;
    std::cout << "\t-h N: Set the workday to N hours." << std::endl;
    std::cout << "\t-w N: Set the work week to N days." << std::endl;
    std::cout << "\t-g N: Set the initial number of goods to N." << std::endl;
    std::cout << "\t-m N: Set the initial number of goods per machine to N." << std::endl;
    std::cout << "\t-r N: Set the initial number of producers to N." << std::endl;
    std::cout << "\t-d N: Set the initial number of distributors to N." << std::endl;
    std::cout << "\t-a N: Set the number of modeled abilities (work skills) to N." << std::endl;
    std::cout << "\t-v N: Set the standard deviation of abilities to N." << std::endl;
    std::cout << "\t-e N: Set the random seed to N." << std::endl;
    std::cout << "\t-s N: Set the annual chance of an agent getting sick." << std::endl;
    std::cout << "\t--init_prices S: How initial prices are set. Options are 'labor_values' and 'equilibrium_prices'." << std::endl;
    std::cout << "\t--production_difficulty N: Set the spectral radius of the I/O requirements matrix. Higher values make surplus generation more difficult." << std::endl;
    std::cout << "\t--consumption_demand N: Set the proportion of value consumed by society relative to the maximum value producable by that society." << std::endl;
    std::cout << "\t-j: Write JSON log traces to stdout." << std::endl;
}

enum class ArgType {
    kTimeSteps,
    kPeople,
    kWorkHours,
    kWorkDays,
    kGoods,
    kGoodsPerMachine,
    kProducers,
    kDistributors,
    kAbilities,
    kAbilityStdDev,
    kSickChance,
    kSeed,
    kProductionDifficulty,
    kConsumptionDemand,
    InitPrices
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
        {"-m", ArgType::kGoodsPerMachine}, {"--products-per-machine", ArgType::kGoodsPerMachine},
        {"-r", ArgType::kProducers}, {"--producers", ArgType::kProducers},
        {"-d", ArgType::kDistributors}, {"--distributors", ArgType::kDistributors},
        {"-a", ArgType::kAbilities}, {"--abilities", ArgType::kAbilities},
        {"-v", ArgType::kAbilityStdDev}, {"--ability-stddev", ArgType::kAbilityStdDev},
        {"-s", ArgType::kSickChance}, {"--sick-chance", ArgType::kSickChance},
        {"-e", ArgType::kSeed}, {"--seed", ArgType::kSeed},
        {"--production_difficulty", ArgType::kProductionDifficulty},
        {"--consumption_demand", ArgType::kConsumptionDemand},
        {"--init_prices", ArgType::InitPrices}
    };


    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
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

        if (valid_args.at(arg) == ArgType::InitPrices) {
            args.init_price_mode = argv[++i];
            continue;
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
            case ArgType::kGoodsPerMachine: {
                if (value <= 0) {
                    error = true;
                } else {
                    args.goods_per_machine = value;
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
                    args.difficulty_of_production = dvalue;
                }
                break;
            }
            case ArgType::kConsumptionDemand: {
                if (dvalue < 0.0 || dvalue >= 1.0) {
                    error = true;
                } else {
                    args.consumption_demand_level = dvalue;
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
            case ArgType::InitPrices: {
                args.init_price_mode = argv[++i];
                if (!valid_init_price_modes.count(args.init_price_mode)) {
                    std::cerr << "Warning! Unknown initial price mode " << args.init_price_mode << ". Defaulting to labor_values" << std::endl;
                }
                continue;
            }
        }
    }
    if (error) {
        print_usage();
        exit(1);
    }
}

int main(int argc, const char ** argv) {
    SimArgs args;
    set_params(argc, argv, args);
    Sim::run(args);
	return EXIT_SUCCESS;
}
