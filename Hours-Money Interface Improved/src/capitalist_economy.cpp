#include <capitalist_economy.h>
#include <capitalist_product.h>
#include <string>
#include <random>
#include <map>

int generate_random_int(int min, int max)
{
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(min, max);
    return dist(gen);
}

double generate_random_double(double min, double max)
{
    static std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<> dist(min, max);
    return dist(gen);
}

void CapitalistEconomy::generate_dependencies(std::map<char, std::vector<char>> &dependencies, int number_of_base_products)
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

void CapitalistEconomy::generate_production_cost_map(std::map<char, double> &production_cost_map, std::map<char, std::vector<char>> &dependencies)
{
    // Random variation factor
    const double RANDOM_MIN = 0.8;
    const double RANDOM_MAX = 1.2;

    // Random variation simulates natural fluctuations in extraction,
    // transport, and market conditions for each base commodity.

    // Each base product’s fixed ratio (defined in values.h) already
    // includes its inherent processing/extraction cost, so no extra
    // base_processing_cost is added here.

    // The hybrid model applies a ±20% random factor to introduce
    // slight variability while preserving realistic cost ratios.
    // These fluctuations will persist across production cycles but remain
    // bounded within the ±20% range of each product’s base cost
    // defined in values.h.
    production_cost_map['A'] = BASE_COST_A; // fixed baseline

    production_cost_map['B'] = BASE_COST_B * generate_random_double(RANDOM_MIN, RANDOM_MAX);
    production_cost_map['C'] = BASE_COST_C * generate_random_double(RANDOM_MIN, RANDOM_MAX);
    production_cost_map['D'] = BASE_COST_D * generate_random_double(RANDOM_MIN, RANDOM_MAX);
    production_cost_map['E'] = BASE_COST_E * generate_random_double(RANDOM_MIN, RANDOM_MAX);
    production_cost_map['F'] = BASE_COST_F * generate_random_double(RANDOM_MIN, RANDOM_MAX);
    production_cost_map['G'] = BASE_COST_G * generate_random_double(RANDOM_MIN, RANDOM_MAX);
    production_cost_map['H'] = BASE_COST_H * generate_random_double(RANDOM_MIN, RANDOM_MAX);

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

void CapitalistEconomy::generate_labor_cost_map(std::map<char, double> &labor_cost_map, std::map<char, double> &production_cost_map)
{
    // LaborCost(P) = ProductionCost(P) × LaborShare(P)

    // This function generates a separate map for human labor costs,
    // excluding raw materials and production costs already included
    // in production_cost_map.

    // Each product’s labor intensity is determined by its production type:
    //   - Automated (0.1–0.4): low human labor input
    //   - Balanced  (0.4–0.7): medium human labor input
    //   - Handcrafted (0.7–1.0): high human labor input

    // Base products (A–H) are generally automated or balanced,
    // while derived products (I–Z) can vary across all three types.

    for (char product = 'A'; product <= 'Z'; ++product)
    {
        // Randomly assign production type
        int production_type = generate_random_int(1, 3); // 1=Automated, 2=Balanced, 3=Handcrafted
        double labor_share = 0.0;

        // Assign labor share based on production type
        if (production_type == 1) // Automated
        {
            labor_share = generate_random_double(0.1, 0.4);
        }
        else if (production_type == 2) // Balanced
        {
            labor_share = generate_random_double(0.4, 0.7);
        }
        else // Handcrafted
        {
            labor_share = generate_random_double(0.7, 1.0);
        }

        // Base products (A–H) are rarely handcrafted, reduce labor intensity if so
        if (product <= 'H' && production_type == 3)
        {
            labor_share *= 0.6; // reduce handcrafted intensity for extraction-based industries
        }

        // Labor cost formula:
        // LaborCost(P) = ProductionCost(P) × LaborShare(P)
        labor_cost_map[product] = production_cost_map[product] * labor_share;
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
