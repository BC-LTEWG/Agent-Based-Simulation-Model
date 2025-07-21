#include "../include/Firm.h"
#include "../include/Worker.h"
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>


Firm::Firm(int id, std::string name) : id(id), name(name), laborTimeHoursSpent(0.0) {
}

int Firm::getId() const {
    return id;
}










