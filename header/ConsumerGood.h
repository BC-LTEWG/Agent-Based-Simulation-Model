#pragma once

#include "Product.h"

struct Good;

struct ConsumerGood : public Product {
    ConsumerGood(Good * good);
    void set_inputs() override; 
    void set_machines() override;
    void log_mean_consumption_frequency();

    Good * corresponding_good = nullptr;
	double mean_consumption_frequency;
    bool paid = true;
};

