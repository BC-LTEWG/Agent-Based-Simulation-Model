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
	static int get_current_time_step();
	static std::vector<Ability> get_all_abilities();
    void time_step();
    void run(int time_steps);

  private:
	static int current_time_step;
	static std::vector<Ability> all_abilities ;
	Society* society;
};
