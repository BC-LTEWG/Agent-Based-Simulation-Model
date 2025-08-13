#include <capitalist_product.h>
#include <string>
#include <random>

// generates a unique name for the new product
void CapitalistProduct::generate_name()
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

// assign a product type out of the 26 available types
void CapitalistProduct::generate_product_type()
{
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(0, 25);

    product_type = 'A' + dist(gen);
}

// assign a random price between 10 and 100
// This value can be changed!
void CapitalistProduct::new_desired_price()
{
    static std::mt19937 gen(std::random_device{}());
    price = std::uniform_real_distribution<>(10, 100)(gen);
}

// new sell factor
void CapitalistProduct::new_sell_factor()
{
    static std::mt19937 gen(std::random_device{}());
    sell_factor = std::uniform_real_distribution<>(0, 1)(gen);
}

// total amount producted between 100 and 1000
// This value can be changed!
void CapitalistProduct::new_total_amount()
{
    static std::mt19937 gen(std::random_device{}());
    total_amount = std::uniform_real_distribution<>(100, 1000)(gen);
}

// Of the price, 10 percent to 90 percent can be surplus
// This value can be changed!
void CapitalistProduct::new_surplus_factor()
{
    static std::mt19937 gen(std::random_device{}());
    surplus_factor = std::uniform_real_distribution<>(0.1, 0.9)(gen);
}