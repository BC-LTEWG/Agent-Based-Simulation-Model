#include "capitalist_company.h"
#include <string>
#include <random>

std::string CapitalistCompany::print_products()
{
    std::string product_names;
    for (const auto &product : products)
    {
        product_names += product.name;
        product_names += " ";
    }
    return product_names;
}

void CapitalistCompany::new_create_factor()
{
    static std::mt19937 gen(std::random_device{}());
    create_factor = std::uniform_real_distribution<>(0, 1)(gen);
}

void CapitalistCompany::new_remove_factor()
{
    static std::mt19937 gen(std::random_device{}());
    remove_factor = std::uniform_real_distribution<>(0, 1)(gen);
}