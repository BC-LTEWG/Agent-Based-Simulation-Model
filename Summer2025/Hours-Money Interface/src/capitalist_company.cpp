#include "capitalist_company.h"
#include "capitalist_economy.h"
#include <string>
#include <random>

CapitalistCompany::CapitalistCompany(CapitalistEconomy *ce)
{
    set_company_name();
    economy = ce;
    economy->companies.push_back(*this);
}

void CapitalistCompany::create_product()
{
    CapitalistProduct p;
    p.set_product_name();
    p.company = this;
    p.set_product_type();
    p.set_desired_price();
    products.push_back(p);
    economy->add_product(p);
}

void CapitalistCompany::set_company_name()
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

std::string CapitalistCompany::print_products()
{
    std::string product_names;
    for (const auto &product : products)
    {
        product_names += product.product_name;
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
