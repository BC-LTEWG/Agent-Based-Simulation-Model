#include "labor_time_company.h"
#include "labor_time_economy.h"
#include <string>
#include <random>

LaborTimeCompany::LaborTimeCompany(LaborTimeEconomy *lte)
{
    set_company_name();
    economy = lte;
    economy->companies.push_back(*this);
}

void LaborTimeCompany::create_product()
{
    LaborTimeProduct p;
    p.set_product_name();
    p.company = this;
    p.set_product_type();
    p.set_labor_time();
    products.push_back(p);
}

void LaborTimeCompany::set_company_name()
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

    company_name = id;
}

std::string LaborTimeCompany::print_products()
{
    std::string product_names;
    for (const auto &product : products)
    {
        product_names += product.product_name;
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