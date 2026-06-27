#pragma once

#include <random>
#include <string>
#include <vector>
#include "Constants.h"
#include "Society.h"

struct SimArgs {
    unsigned int time_steps = NUM_SIM_RUNS;
    unsigned int num_people = STARTING_NUM_PEOPLE;
    unsigned int work_hours_daily = INITIAL_WORK_HOURS_DAILY;
    unsigned int work_days_weekly = INITIAL_WORK_DAYS_WEEKLY;
    unsigned int num_goods = STARTING_NUM_GOODS;
    unsigned int goods_per_machine = STARTING_GOODS_PER_MACHINE;
    unsigned int num_producers = STARTING_NUM_PRODUCERS;
    unsigned int num_distributors = STARTING_NUM_DISTRIBUTORS;
    double sickness_chance = ANNUAL_SICKNESS_CHANCE;
    unsigned int num_abilities = NUM_ABILITIES;
    double ability_stddev = ABILITY_STDDEV;
    double difficulty_of_production = DEFAULT_PRODUCTION_DIFFICULTY;
    double consumption_demand_level = DEFAULT_CONSUMPTION_DEMAND;
    int public_sector_expansion_period = PUBLIC_SECTOR_EXPANSION_PERIOD;
    std::string init_price_mode = "labor_values";
    bool json = false;
    unsigned int seed = 0;
    bool fixed_seed = false;
};

class Sim {
    public:
        static Sim& get_instance();
        static void run(SimArgs& args);
        static unsigned int get_num_people();
        static unsigned int get_work_hours_daily();
        static unsigned int get_work_days_weekly();
        static unsigned int get_num_goods();
        static unsigned int get_goods_per_machine();
        static unsigned int get_num_producers();
        static unsigned int get_num_distributors();
        static unsigned int get_num_abilities();
        static double get_ability_stddev();
        static double get_annual_sickness_chance();
        static double get_difficulty_of_production();
        static double get_product_consumption_mult();
        static int get_public_sector_expansion_period();
        static std::string get_initial_price_mode();
        static bool does_json();
        static int get_current_time_step();
        static std::mt19937& get_random_generator();
        void set_params(SimArgs& args);
    private:
        Sim();
        void run();
        SimArgs args;
        std::random_device rd;
        unsigned int seed = 0;
        std::mt19937 gen;
        int current_time_step;
        Society * society;
};
