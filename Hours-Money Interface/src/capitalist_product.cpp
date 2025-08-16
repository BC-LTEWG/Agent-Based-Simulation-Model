#include <capitalist_product.h>
#include <string>
#include <random>

// generates a unique name for the new product
void CapitalistProduct::set_product_name()
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
void CapitalistProduct::set_product_type()
{
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(0, 25);

    product_type = 'A' + dist(gen);
}

// assign a random price between 10 and 100
// This value can be changed!
void CapitalistProduct::set_desired_price()
{
    static std::mt19937 gen(std::random_device{}());
    price = std::uniform_real_distribution<>(10, 100)(gen);
}

// new sell factor
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

// Of the price, 10 percent to 90 percent can be surplus
// 0.1 to 0.9 is an arbitrary value.
void CapitalistProduct::set_surplus_factor()
{
    static std::mt19937 gen(std::random_device{}());
    surplus_factor = std::uniform_real_distribution<>(0.1, 0.9)(gen);
}