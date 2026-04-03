#include <iostream>

#include "Sim.h"

void print_usage() {
    std::cout << "Usage: sim [<args> ...]" << std::endl;
    std::cout << "\t-n N: Run the simulation for N time steps." << std::endl;
    std::cout << "\t-j: Write log traces to stdin in JSON format throughout "
        "execution." << std::endl;
}

void set_params(int argc, const char ** argv, SimArgs& args) {
    for (int i = 0; i < argc; ++i) {
        unsigned j = 0;
        if (argv[i][j] == '-') {
            ++j;
            if (argv[i][j] == 'n') {
                ++i;
                long num_time_steps = strtol(argv[i], NULL, 10);
                if (num_time_steps) {
                    args.time_steps = static_cast<unsigned int>(num_time_steps);
                }
                continue;
            } else {
                while (argv[i][j]) {
                    switch (argv[i][j]) {
                        case 'j':
                            args.json = true;
                            break;
                        case 'h':
                            print_usage();
                            exit(EXIT_SUCCESS);
                    }
                    ++j;
                }
            }
        }
    }
}

int main(int argc, const char ** argv) {
    SimArgs args;
    set_params(argc, argv, args);
    Sim::run(args);
	return EXIT_SUCCESS;
}
