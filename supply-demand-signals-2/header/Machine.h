#pragma once
#include <string>
#include <vector>


class Machine {
    public:
        std::string name;
        int lifetime;
        Machine(const std::string& name, int lifetime) : name(name), lifetime(lifetime) {}
        std::vector<Firm&> addMachinetoFirm(Firm& f, std::vector<Machine&> machines);
};


