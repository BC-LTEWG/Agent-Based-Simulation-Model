#include <capitalist_economy.h>
#include <capitalist_product.h>
#include "random_utils.h"
#include <string>
#include <iostream>
#include <random>
#include <map>

void CapitalistEconomy::generate_dependencies(int number_of_base_products)
{
    for (int i = 0; i < 18; i++)
    {
        char current_char = 'I' + i;
        int number_of_dependencies = generate_random_int(2, 5);
        std::vector<char> current_product_type_dependencies;

        for (int j = 0; j < number_of_dependencies; j++)
        {
            int selected_dependency = generate_random_int(0, number_of_base_products - 1);
            char selected_dependency_char = 'A' + selected_dependency;
            if (std::find(current_product_type_dependencies.begin(), current_product_type_dependencies.end(), selected_dependency_char) != current_product_type_dependencies.end())
            {
                j--;
            }
            else
            {
                current_product_type_dependencies.push_back(selected_dependency_char);
            }
        }

        dependencies[current_char] = current_product_type_dependencies;
    }
}

void CapitalistEconomy::generate_production_cost_map()
{
    // Production cost of base products

    // Each base product’s fixed ratio (defined in values.h) already
    // includes its inherent processing/extraction cost, so no extra
    // base_processing_cost is added here.
    production_cost_map['A'] = BASE_COST_A; // fixed baseline

    production_cost_map['B'] = BASE_COST_B;
    production_cost_map['C'] = BASE_COST_C;
    production_cost_map['D'] = BASE_COST_D;
    production_cost_map['E'] = BASE_COST_E;
    production_cost_map['F'] = BASE_COST_F;
    production_cost_map['G'] = BASE_COST_G;
    production_cost_map['H'] = BASE_COST_H;

    // Production cost for dependencies
    // Total production cost = base processing cost + sum of dependent base product cost (weight/percentage of the base product times production cost of base product)
    for (int i = 0; i < 18; i++)
    {
        char current_product_type = 'I' + i;
        double base_processing_cost = generate_random_double(0.2, 0.5);

        // generate the sum of dependent base product cost
        double sum_of_dependent_base_product_cost = 0;
        double total_weight = 0;
        std::vector<double> specific_weights;

        for (char dependency : dependencies[current_product_type])
        {
            double specific_weight = generate_random_double(0.1, 1);
            specific_weights.push_back(specific_weight);
            total_weight += specific_weight;
        }

        int current_index = 0;
        for (char dependency : dependencies[current_product_type])
        {
            // Normalize each weight by dividing it by the total weight
            sum_of_dependent_base_product_cost += ((specific_weights[current_index] / total_weight) * production_cost_map[dependency]);
            current_index++;
        }

        production_cost_map[current_product_type] = base_processing_cost + sum_of_dependent_base_product_cost;
    }
}

void CapitalistEconomy::generate_labor_cost_map()
{
    // Labor cost of base products
    labor_cost_map['A'] = BASE_LABOR_A; // fixed baseline

    labor_cost_map['B'] = BASE_LABOR_B;
    labor_cost_map['C'] = BASE_LABOR_C;
    labor_cost_map['D'] = BASE_LABOR_D;
    labor_cost_map['E'] = BASE_LABOR_E;
    labor_cost_map['F'] = BASE_LABOR_F;
    labor_cost_map['G'] = BASE_LABOR_G;
    labor_cost_map['H'] = BASE_LABOR_H;

    // Derived product labor intensities (I–Z)

    // Each derived product’s total labor cost is composed of:
    // 1. A base processing labor (direct labor intensity)
    // 2. The weighted sum of the labor intensities of its dependent base products (indirect labor intensity)

    // Formula:
    //   L_total(product) = L_direct + Σ (weight_i / total_weight) * L(dependency_i)
    // Units: labor-hours per unit of derived product
    for (int i = 0; i < 18; i++)
    {
        char current_product_type = 'I' + i;

        // Direct labor intensity for the derived product (human handling, assembly, etc.)
        double base_processing_labor = generate_random_double(0.3, 0.7);

        // Calculate total labor contributed by dependent base products
        double sum_of_dependent_labor = 0.0;
        double total_weight = 0.0;
        std::vector<double> specific_weights;

        // Generate random weights for each dependency
        for (char dependency : dependencies[current_product_type])
        {
            double specific_weight = generate_random_double(0.1, 1.0);
            specific_weights.push_back(specific_weight);
            total_weight += specific_weight;
        }

        // Normalize sum of dependency labor intensities
        int current_index = 0;
        for (char dependency : dependencies[current_product_type])
        {
            double normalized_weight = specific_weights[current_index] / total_weight;
            sum_of_dependent_labor += normalized_weight * labor_cost_map[dependency];
            current_index++;
        }

        // Total labor-hours per unit of product (direct + indirect)
        double total_labor_intensity = base_processing_labor + sum_of_dependent_labor;

        // Store the final result
        labor_cost_map[current_product_type] = total_labor_intensity;
    }
}

void CapitalistEconomy::generate_total_cost_map(double melt)
{
    // Total cost per product
    // Total cost = Production cost (machinery + raw materials) + Labor cost * MELT
    // Units:
    // - Production cost: CE currency per unit
    // - Labor cost: labor-hours per unit
    // - MELT: CE currency per labor-hour
    // - Total cost: CE currency per unit
    for (auto &entry : production_cost_map)
    {
        char product = entry.first;
        double production_cost = entry.second;
        double labor_hours = labor_cost_map[product];

        double total_cost = production_cost + (labor_hours * melt);

        total_cost_map[product] = total_cost;
    }
}

void CapitalistEconomy::print_dependencies()
{
    std::cout << " Dependency map printed below " << std::endl;

    for (int c = 0; c < 26; c++)
    {
        char current_char = 'A' + c;
        std::cout << "Product type: " << current_char;
        std::cout << " Dependencies of current product: ";
        for (int length = 0; length < dependencies[current_char].size(); length++)
        {
            std::cout << dependencies[current_char][length] << " ";
        }
        std::cout << std::endl;
    }
}

void CapitalistEconomy::print_production_cost_map()
{
    std::cout << " Production cost map printed below " << std::endl;

    for (int c = 0; c < 26; c++)
    {
        char current_char = 'A' + c;
        std::cout << "Product type: " << current_char;
        std::cout << " Production cost of current product: " << production_cost_map[current_char] << std::endl;
    }
}

void CapitalistEconomy::print_labor_cost_map()
{
    std::cout << " Labor cost map printed below " << std::endl;

    for (int c = 0; c < 26; c++)
    {
        char current_char = 'A' + c;
        std::cout << "Product type: " << current_char;
        std::cout << " Labor cost of current product: " << labor_cost_map[current_char] << std::endl;
    }
}

void CapitalistEconomy::print_total_cost_map()
{
    std::cout << " Total cost map printed below " << std::endl;

    for (int c = 0; c < 26; c++)
    {
        char current_char = 'A' + c;
        std::cout << "Product type: " << current_char;
        std::cout << " Total cost of current product: " << total_cost_map[current_char] << std::endl;
    }
}

// New gdp per capita or mean per capita every year
void CapitalistEconomy::new_earnings_per_capita()
{
    static std::mt19937 gen(std::random_device{}());
    earnings_per_capita *= std::uniform_real_distribution<>(0.9, 1.1)(gen);
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
    melt = earnings_per_capita / work_hours;
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
    std::uniform_real_distribution<double> dist(0, 0.5);
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

// removing the product from the market system
// Adjusting the market price based on the product type
void CapitalistEconomy::remove_product(CapitalistProduct &product)
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

    // Remove the incoming product in the batch
    same_type.erase(
        std::remove_if(
            same_type.begin(),
            same_type.end(),
            [&](const CapitalistProduct &p)
            {
                return &p == &product; // compare by memory address
            }),
        same_type.end());

    if (same_type.empty())
    {
        market_price.erase(product.product_type);
        return;
    }

    // One random market adjustment factor for the remaining type batch
    static std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<double> dist(0, 0.5);
    double market_factor = dist(gen);

    // To calculate the average market price for the entire product type
    double total_market_price = 0;

    // Adjust all same-type products together
    for (CapitalistProduct &p : same_type)
    {
        double original_cost = p.price - (p.price * p.surplus_factor);
        p.price += p.price * p.surplus_factor * market_factor;
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
