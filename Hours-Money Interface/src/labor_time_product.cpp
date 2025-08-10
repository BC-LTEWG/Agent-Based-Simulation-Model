#include <labor_time_product.h>
#include <capitalist_economy.h>
#include <capitalist_product.h>
#include <string>
#include <random>

// generates a unique name for the new product
void LaborTimeProduct::generate_name()
{
    static const std::string chars =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    static int length = 20;

    static std::mt19937 rng(std::random_device{}()); // Random number engine
    static std::uniform_int_distribution<> dist(0, chars.size() - 1);

    std::string id;
    id.reserve(length);

    for (size_t i = 0; i < length; ++i)
    {
        id += chars[dist(rng)];
    }

    name = id;
}

// assign a random labor time between 10 and 40
// This value can be changed!
void LaborTimeProduct::new_labor_time()
{
    static std::mt19937 gen(std::random_device{}());
    labor_time = std::uniform_real_distribution<>(10, 40)(gen);
}

// how much will be product be in the destined country
void LaborTimeProduct::calculate_sell_price(CapitalistEconomy economy)
{
    // I am checking if the product exists in the market
    // If the product doesn't exist, then we use melt
    // But I am assigning almost unique names so I doubt there would be an exist product
    // That's why I think the whole market system
    // should either be removed or I should be giving names that can be repeated
    for (CapitalistProduct product : economy.market)
    {
        if (product.name == name)
        {
            sell_price = product.price * 0.9; // 0.9 of the market price?
            return;
        }
    }
    sell_price = labor_time * economy.melt;
}

// new sell factor
void LaborTimeProduct::new_sell_factor()
{
    static std::mt19937 gen(std::random_device{}());
    sell_factor = std::uniform_real_distribution<>(0, 1)(gen);
}

// total amount producted between 100 and 1000
// This value can be changed!
void LaborTimeProduct::new_total_amount()
{
    static std::mt19937 gen(std::random_device{}());
    total_amount = std::uniform_real_distribution<>(100, 1000)(gen);
}

// Of the converted price, we can add 10 percent to 90 percent of the value to be surplus
// This value can be changed!
void LaborTimeProduct::new_surplus_factor()
{
    static std::mt19937 gen(std::random_device{}());
    surplus_factor = std::uniform_real_distribution<>(0.1, 0.9)(gen);
}