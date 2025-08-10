#include <capitalist_economy.h>
#include <capitalist_product.h>
#include <string>
#include <random>

// New gdp per capita or mean per capita every year
void CapitalistEconomy::new_indicator()
{
    static std::mt19937 gen(std::random_device{}());
    economic_indicator *= std::uniform_real_distribution<>(0.9, 1.1)(gen);
}

// New work hours every year, between 800 and 2400
void CapitalistEconomy::new_work_hours()
{
    if (work_hours >= 1000 && work_hours <= 2000)
    {
        static std::mt19937 gen(std::random_device{}());
        work_hours *= std::uniform_real_distribution<>(0.8, 1.2)(gen);
    }
    else if (work_hours < 1000 && work_hours >= 800)
    {
        static std::mt19937 gen(std::random_device{}());
        work_hours *= std::uniform_real_distribution<>(1, 1.2)(gen);
    }
    else if (work_hours > 2000 && work_hours <= 2400)
    {
        static std::mt19937 gen(std::random_device{}());
        work_hours *= std::uniform_real_distribution<>(0.8, 1)(gen);
    }
}

// New melt for this specific economy in this specifc year
void CapitalistEconomy::new_melt()
{
    melt = economic_indicator / work_hours;
}

// Converting the desired price to the market price
void CapitalistEconomy::add_product(CapitalistProduct &product)
{
    static std::mt19937 gen(std::random_device{}());
    product.price *= std::uniform_real_distribution<>(0.8, 1.2)(gen); // 0.8 to 1.2 is a random range, it can change
    market.push_back(product);
}

// Remove item with the same name
void CapitalistEconomy::remove_product(const std::string &name)
{
    for (auto it = market.begin(); it != market.end(); ++it)
    {
        if (it->name == name)
        {
            market.erase(it);
            break;
        }
    }
}

void CapitalistEconomy::new_create_factor()
{
    static std::mt19937 gen(std::random_device{}());
    create_factor = std::uniform_real_distribution<>(0, 1)(gen);
}

void CapitalistEconomy::new_remove_factor()
{
    static std::mt19937 gen(std::random_device{}());
    remove_factor = std::uniform_real_distribution<>(0, 1)(gen);
}
