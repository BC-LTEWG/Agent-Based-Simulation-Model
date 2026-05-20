#pragma once

#include "Product.h"

struct ConsumerGood;

struct Good : public Product {
    Good();
    void set_corresponding_consumer_good(ConsumerGood * consumer_good);

    ConsumerGood * corresponding_consumer_good = nullptr;
};
