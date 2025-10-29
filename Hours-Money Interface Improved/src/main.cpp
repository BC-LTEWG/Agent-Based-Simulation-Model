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
    // Build the economies
    CapitalistEconomy ce;
    ce.name = "ce";

    ce.generate_dependencies(NUMBER_OF_BASE_PRODUCTS);
    ce.generate_production_cost_map();
    ce.generate_labor_cost_map();

    ce.print_dependencies();
    ce.print_production_cost_map();
    ce.print_labor_cost_map();

    return 0;
}

// This is my main for the previous project
// The structure of this main is reusable in future simulation
/*
int main()
{
    // Build the two economies
    CapitalistEconomy ce;
    ce.name = "ce";
    double ce_initial_amount = ce.curreny_amount;
    LaborTimeEconomy lte;
    lte.name = "lte";
    double lte_initial_amount = lte.currency_amount;

    // Build three companies initially each with one product for capitalist economy
    for (int i = 0; i < 3; i++)
    {
        CapitalistCompany cc = CapitalistCompany(&ce);
        cc.create_product();
    }

    // Build three companies initially each with one product for labor time economy
    for (int i = 0; i < 3; i++)
    {
        LaborTimeCompany ltc = LaborTimeCompany(&lte);
        ltc.create_product();
    }

    // Vectors for data recording
    std::vector<double> ce_data;
    ce_data.resize(10);
    std::vector<double> lte_data;
    lte_data.resize(10);

    // Start recording data
    for (int i = 0; i < 10; i++)
    {
        // New melt for each year
        ce.new_melt();

        // Capitalist economy decides whether to create companies
        ce.new_create_factor();
        double ce_decision_for_create = generate_number(0, 1);

        if (ce_decision_for_create <= ce.create_factor)
        {
            CapitalistCompany cc = CapitalistCompany(&ce);
        }

        // Capitalist economy decides whether to remove companies
        ce.new_remove_factor();
        double ce_decision_for_remove = generate_number(0, 1);

        if (ce_decision_for_remove <= ce.remove_factor && !ce.companies.empty())
        {
            int index = generate_number(0, ce.companies.size() - 1);

            for (CapitalistProduct &p : ce.companies[index].products)
            {
                ce.remove_product(p);
            }

            ce.companies.erase(ce.companies.begin() + index);
        }

        // Labor Time economy decides whether to create companies
        lte.new_create_factor();
        double lte_decision_for_create = generate_number(0, 1);

        if (lte_decision_for_create <= lte.create_factor)
        {
            LaborTimeCompany ltc = LaborTimeCompany(&lte);
        }

        // Labor Time economy decides whether to remove companies
        lte.new_remove_factor();
        double lte_decision_for_remove = generate_number(0, 1);

        if (lte_decision_for_remove <= lte.remove_factor && !lte.companies.empty())
        {
            lte.companies.erase(lte.companies.begin() + generate_number(0, lte.companies.size() - 1));
        }

        // Capitalist companies decide to whether to create products
        for (CapitalistCompany &cc : ce.companies)
        {
            cc.new_create_factor();
            double cc_decision_for_create = generate_number(0, 1);

            if (cc_decision_for_create <= cc.create_factor)
            {
                cc.create_product();
            }
        }

        // Capitalist companies decide to whether to remove products
        for (CapitalistCompany &cc : ce.companies)
        {
            cc.new_remove_factor();
            double cc_decision_for_remove = generate_number(0, 1);

            if (cc_decision_for_remove <= cc.remove_factor && !cc.products.empty())
            {
                int index = generate_number(0, cc.products.size() - 1);

                cc.economy->remove_product(cc.products[index]);

                cc.products.erase(cc.products.begin() + index);
            }
        }

        // Labor time companies decide to whether to create products
        for (LaborTimeCompany &ltc : lte.companies)
        {
            ltc.new_create_factor();
            double ltc_decision_for_create = generate_number(0, 1);

            if (ltc_decision_for_create <= ltc.create_factor)
            {
                ltc.create_product();
            }
        }

        // Labor time companies decide to whether to remove products
        for (LaborTimeCompany &ltc : lte.companies)
        {
            ltc.new_remove_factor();
            double ltc_decision_for_remove = generate_number(0, 1);

            if (ltc_decision_for_remove <= ltc.remove_factor && !ltc.products.empty())
            {
                ltc.products.erase(ltc.products.begin() + generate_number(0, ltc.products.size() - 1));
            }
        }

        // LTE companies selling products
        double lte_total_revenue = 0;

        for (LaborTimeCompany &ltc : lte.companies)
        {
            for (LaborTimeProduct &ltp : ltc.products)
            {
                ltp.export_to_destined_economy(ce);
                lte_total_revenue += ltp.sale_price * ltp.portion_sold_in_export * ltp.quantity_produced_for_export;
            }
        }

        lte.currency_amount += lte_total_revenue;
        ce.curreny_amount -= lte_total_revenue;

        // CE companies selling products
        double ce_total_revenue = 0;

        for (CapitalistCompany &cc : ce.companies)
        {
            for (CapitalistProduct &cp : cc.products)
            {
                cp.export_to_destined_economy(lte);
                ce_total_revenue += cp.price * cp.portion_sold_in_export * cp.quantity_produced_for_export;
            }
        }

        ce.curreny_amount += ce_total_revenue;
        lte.currency_amount -= ce_total_revenue;

        // Record money amount
        ce_data[i] = ce.curreny_amount;
        lte_data[i] = lte.currency_amount;

        // CE adjusting for next cycle
        ce.new_earnings_per_capita();
        ce.new_work_hours();
        ce.adjust_market_price();
    }

    // Open output CSV file
    std::ofstream outfile("data/output.csv");
    if (!outfile.is_open())
    {
        std::cerr << "Failed to open CSV file for writing.\n";
        return 1;
    }

    outfile << std::fixed << std::setprecision(2);

    // Write CSV header
    outfile << "country_name,"
            << "initial_value,"
            << "year_one,year_two,year_three,year_four,year_five,"
            << "year_six,year_seven,year_eight,year_nine,year_ten\n";

    // Write first row
    outfile << ce.name << "," << ce_initial_amount;
    for (double val : ce_data)
        outfile << "," << val;
    outfile << "\n";

    // Write second row
    outfile << lte.name << "," << lte_initial_amount;
    for (double val : lte_data)
        outfile << "," << val;
    outfile << "\n";

    outfile.close();
    std::cout << "Simulation progress complete! Results in the 'output.csv' file.\n";

    return 0;
}
*/