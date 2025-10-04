#pragma once
#include <string>
#include <vector>


struct Machine {
    public:
        const std::string name;
        const int lifetime_hours;
        int hopurs_used;
        Machine(const std::string& name, int lifetime) : name(name), lifetime(lifetime) {}

        Machine& machineUsage(std::string machingName) {};


};
