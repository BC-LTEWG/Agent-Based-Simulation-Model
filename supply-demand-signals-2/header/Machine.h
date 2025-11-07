#pragma once

#include <string>
#include <vector>

class Machine {
  public:
    std::string name;
    const int lifetime;
    int hours_used;
    
    Machine(const std::string& name, int lifetime);
    int get_hours_used();
};
