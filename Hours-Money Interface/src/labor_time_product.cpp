#include <labor_time_product.h>
#include <capitalist_economy.h>
#include <capitalist_product.h>
#include <string>
#include <random>
#include <map>

// generates a unique name for the new product
void LaborTimeProduct::set_product_name()
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

    product_name = id;
}

// assign a product type out of the 26 available types
void LaborTimeProduct::set_product_type()
{
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(0, 25);

    product_type = 'A' + dist(gen);
}

// assign a random labor time between 10 and 40
// This value can be changed!
void LaborTimeProduct::set_labor_time()
{
    static std::mt19937 gen(std::random_device{}());
    labor_time = std::uniform_real_distribution<>(10, 40)(gen);
}

// how much will be product be in the destined country
void LaborTimeProduct::calculate_sell_price(const CapitalistEconomy &economy)
{
    // I am checking if the product type exists in the market
    // If the product doesn't exist, then we use melt
    for (const std::pair<char, double> &product : economy.market_price)
    {
        if (product.first == product_type)
        {
            sell_price = product.second * 0.9; // 0.9 of the market price?
            return;
        }
    }
    sell_price = labor_time * economy.melt + (labor_time * economy.melt) / (1 - surplus_factor);
}

// new portion sold
void CapitalistProduct::set_portion_sold_in_export()
{
    static std::mt19937 gen(std::random_device{}());
    portion_sold_in_export = std::uniform_real_distribution<>(0, 1)(gen);
}

// total amount producted between 100 and 1000
// This value can be changed!
void CapitalistProduct::set_quantity_produced_for_export()
{
    static std::mt19937 gen(std::random_device{}());
    quantity_produced_for_export = std::uniform_real_distribution<>(100, 1000)(gen);
}

// Of the converted price, we can add 10 percent to 90 percent of the value to be surplus
// This value can be changed!
void LaborTimeProduct::set_surplus_factor()
{
    static std::mt19937 gen(std::random_device{}());
    surplus_factor = std::uniform_real_distribution<>(0.1, 0.9)(gen);
}