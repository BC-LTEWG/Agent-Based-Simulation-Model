#include <iostream>

#include "Sim.h"

bool arg_present(
        const std::string arg,
        const int argc,
        const char ** const argv
        ) {
    for (int i = 0; i < argc; ++i) {
        if ("--" + arg == argv[i]) {
            return true;
        }
    }
    return false;
}

int main(int argc, const char ** argv) {
	Sim simulation;
    simulation.set_trace_logging(arg_present("trace", argc, argv));
    simulation.set_write_data(arg_present("write", argc, argv));
    simulation.run(1000);
	return EXIT_SUCCESS;
}
