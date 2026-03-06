#include "Constants.h"
#include "Logger.h"
#include "Sim.h"

Sim& Sim::get_instance() {
    static Sim sim;
    return sim;
}

void Sim::run(SimArgs& args) {
    Sim& sim = get_instance();
    sim.set_params(args);
    sim.run();
}

Sim::Sim() :
    gen{rd()}
{}

bool Sim::does_csv() {
    return get_instance().args.csv;
}

bool Sim::does_db() {
    return get_instance().args.db;
}

bool Sim::does_json() {
    return get_instance().args.json;
}

void Sim::set_params(SimArgs& args) {
    this->args = args;
}

std::random_device& Sim::get_random_device() {
    return get_instance().rd;
}

std::mt19937& Sim::get_random_generator() {
    return get_instance().gen;
}

unsigned int Sim::get_current_time_step() {
	return get_instance().current_time_step;
}

void Sim::run() {
    society = Society::get_instance();
    for (std::size_t i = 0; i < args.time_steps; ++i) {
        society->on_time_step();
        current_time_step++;
    }
    if (args.csv) {
        Logger::get_instance()->write_data();
    }
}
