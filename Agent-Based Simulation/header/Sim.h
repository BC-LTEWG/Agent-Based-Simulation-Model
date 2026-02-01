#pragma once

#include <random>
#include <string>
#include <vector>

#include "Society.h"

class Sim {
  public:
	static std::random_device rd;
	static std::mt19937 gen;
    Sim();
    static bool is_trace_logging();
    static bool is_writing_data();
    static void set_trace_logging(bool should_log);
    static void set_write_data(bool should_log);
	static int get_current_time_step();
    void time_step();
    void run(int time_steps);

  private:
    static bool trace_logging;
    static bool write_data;
	static int current_time_step;
	Society * society;
};
