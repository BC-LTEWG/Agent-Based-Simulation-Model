#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Person.h"

struct Good;
struct Machine;

struct Product {
    enum ProductType {
        kTypeGood,
        kTypeMachine,
        kTypeConsumerGood,
        kTypeUnknown
    };
    Product();
    virtual void set_inputs();
    virtual void set_machines();

    int id;
    std::string product_name;
    ProductType product_type;
    double price_per_unit;
    double labor_value;
    std::vector<Machine *> machines_needed;
    std::unordered_map<Good *, double> inputs_per_unit;
    double living_labor_per_unit;
    std::vector<Ability *> required_abilities;
};

