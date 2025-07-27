#include <iostream>
#include <vector>
#include "values.h"
#include "data_loader.h"
#include "product_export.h"
#include "product_import.h"
#include "capitalist_economy.h"
#include "labortime_economy.h"
#include "lte_export_gdp_nominal.h"
#include "lte_export_gdp_ppp.h"
#include "lte_export_median.h"
#include "lte_export_mean.h"
#include "lte_import_gdp_nominal.h"
#include "lte_import_gdp_ppp.h"
#include "lte_import_median.h"
#include "lte_import_mean.h"

using namespace std;

int main()
{
    // Load the CSV data into DataLoader
    DataLoader::loadData("scripts/data.csv");

    // Create a vector of CapitalistEconomy
    vector<CapitalistEconomy> capitalist_economies;
    int id_counter = 1;

    // Iterate through the countries stored in DataLoader using getAllData()
    for (const auto &countryEntry : DataLoader::getAllData())
    {
        CapitalistEconomy ce{};
        ce.ce_id = id_counter++;
        ce.economy_name = countryEntry.first;

        // Assign GDP values (other fields remain INVALID_VALUE)
        auto it_nominal = countryEntry.second.find("GDP_NOMINAL");
        if (it_nominal != countryEntry.second.end())
            ce.gdp_nominal_per_capita = it_nominal->second;

        auto it_ppp = countryEntry.second.find("GDP_PPP");
        if (it_ppp != countryEntry.second.end())
            ce.gdp_ppp_per_capita = it_ppp->second;

        capitalist_economies.push_back(ce);
    }

    // Print out all economies
    cout << capitalist_economies.size() << " capitalist economies loaded:\n";
    for (const auto &ce : capitalist_economies)
    {
        cout << ce.ce_id << " - " << ce.economy_name
             << " | GDP Nominal: " << ce.gdp_nominal_per_capita
             << " | GDP PPP: " << ce.gdp_ppp_per_capita
             << "\n";
    }

    return 0;
}
