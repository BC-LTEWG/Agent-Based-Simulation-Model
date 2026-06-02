#pragma once

#include "Product.h"

struct ConsumerGood;

struct Good : public Product {
    Good();
    ConsumerGood * corresponding_consumer_good = nullptr;
};
