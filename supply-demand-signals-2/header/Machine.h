#pragma once
#include <string>
#include <vector>


class Machine {
    public:
        std::string name;
        const int lifetime;
        int hoursUsed; 
        Machine(const std::string& name, int lifetime) : name(name), lifetime(lifetime), hoursUsed(0) {}
        int getHoursUsed() {return hoursUsed;};

};

