#include <iostream>
#include <vector>
#include <random>
#include "capitalist_economy.h"
#include "capitalist_company.h"
#include "capitalist_product.h"
#include "labor_time_economy.h"
#include "labor_time_company.h"
#include "labor_time_product.h"

#include "random_utils.h"
#include "values.h"
#include <iomanip>
#include <fstream>

using namespace std;

int main()
{
    // Build the capitalist economy
    CapitalistEconomy ce;
    ce.name = "ce";

    // Generate the dependency structure (A-H -> I-Z)
    ce.generate_dependencies(NUMBER_OF_BASE_PRODUCTS);

    // Generate cost maps
    ce.generate_production_cost_map();
    ce.generate_labor_cost_map();

    // Compute MELT (Monetary Expression of Labor Time)
    // MELT = earnings_per_capita / work_hours
    ce.new_melt();
    cout << "MELT (monetary value per labor-hour): " << ce.melt << endl;

    // Generate total cost map using MELT
    ce.generate_total_cost_map(ce.melt);

    // Print all results
    ce.print_dependencies();
    ce.print_production_cost_map();
    ce.print_labor_cost_map();
    ce.print_total_cost_map();

    return 0;
}