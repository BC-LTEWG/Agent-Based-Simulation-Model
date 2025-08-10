#include "labor_time_company.h"
#include <string>
#include <random>

std::string LaborTimeCompany::print_products()
{
    std::string product_names;
    for (const auto &product : products)
    {
        product_names += product.name;
        product_names += " ";
    }
    return product_names;
}

void LaborTimeCompany::new_create_factor()
{
    static std::mt19937 gen(std::random_device{}());
    create_factor = std::uniform_real_distribution<>(0, 1)(gen);
}

void LaborTimeCompany::new_remove_factor()
{
    static std::mt19937 gen(std::random_device{}());
    remove_factor = std::uniform_real_distribution<>(0, 1)(gen);
}