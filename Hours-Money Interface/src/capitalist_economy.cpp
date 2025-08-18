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
// Adjusting the market price based on the product type
void CapitalistEconomy::add_product(CapitalistProduct &product)
{
    // Split market into remaining + same_type
    std::vector<CapitalistProduct> same_type;
    same_type.reserve(8); // small guess to reduce reallocs

    std::vector<CapitalistProduct> remaining;
    remaining.reserve(market.size());

    for (CapitalistProduct &p : market)
    {
        if (p.product_type == product.product_type)
        {
            same_type.push_back(std::move(p));
        }
        else
        {
            remaining.push_back(std::move(p));
        }
    }
    market.swap(remaining); // market now holds only the remaining products

    // Include the incoming product in the batch
    same_type.push_back(product);

    // One random market factor for the entire type batch
    static std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<double> dist(0, 1);
    double market_factor = dist(gen);

    // To calculate the average market price for the entire product type
    double total_market_price = 0;

    // Adjust all same-type products together
    for (CapitalistProduct &p : same_type)
    {
        double original_cost = p.price - (p.price * p.surplus_factor);
        p.price -= p.price * p.surplus_factor * market_factor;
        total_market_price += p.price;
        p.surplus_factor = 1.0 - (original_cost / p.price);
    }

    // Push adjusted batch back to market
    market.insert(market.end(),
                  std::make_move_iterator(same_type.begin()),
                  std::make_move_iterator(same_type.end()));

    market_price[product.product_type] = (total_market_price / same_type.size());
}

// Adjusts the market price of all product types that's available
void CapitalistEconomy::adjust_market_price()
{
    for (int i = 0; i < 26; i++)
    {
        char type = 'A' + i;
        std::vector<CapitalistProduct> type_list;
        std::vector<CapitalistProduct> remaining;
        for (CapitalistProduct &product : market)
        {
            if (product.product_type == type)
            {
                type_list.push_back(std::move(product));
            }
            else
            {
                remaining.push_back(std::move(product));
            }
        }
        market.swap(remaining);
        if (type_list.size() == 0)
        {
            continue;
        }
        else
        {
            double total_market_price = 0;
            static std::mt19937 gen(std::random_device{}());
            std::uniform_real_distribution<double> dist(0.5, 1.5); // Randomly adjusts the market price, this range can change.
            double market_adjustment = dist(gen);

            // Adjust all same-type products together
            for (CapitalistProduct &p : type_list)
            {
                double surplus = p.price * p.surplus_factor;
                double original_cost = p.price - surplus;
                p.price = original_cost + surplus * market_adjustment;
                total_market_price += p.price;
                p.surplus_factor = 1.0 - (original_cost / p.price);
            }

            // Push adjusted batch back to market
            market.insert(market.end(),
                          std::make_move_iterator(type_list.begin()),
                          std::make_move_iterator(type_list.end()));

            market_price[type] = (total_market_price / type_list.size());
        }
    }
}

// Remove item with the same name
void CapitalistEconomy::remove_product(const std::string &name)
{
    for (auto it = market.begin(); it != market.end(); ++it)
    {
        if (it->product_name == name)
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
