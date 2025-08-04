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
#include "lte_export_mean.h"
#include "lte_import_gdp_nominal.h"
#include "lte_import_gdp_ppp.h"
#include "lte_import_mean.h"
#include <fstream>

using namespace std;

int main()
{
    // Load the CSV data into DataLoader
    DataLoader::loadData("scripts/data.csv");

    // Create a vector of CapitalistEconomy
    vector<CapitalistEconomy> capitalist_economies;
    int id_counter = 1;

    // Iterate through the countries in the order they appear in the CSV
    for (const string &country : DataLoader::getCountryOrder())
    {
        CapitalistEconomy ce{};
        ce.ce_id = id_counter++;
        ce.economy_name = country;

        // Assign values
        ce.gdp_nominal_per_capita = DataLoader::getValue(country, "GDP_Nominal_Per_Capita");
        ce.gdp_ppp_per_capita = DataLoader::getValue(country, "GDP_PPP_Per_Capita");
        ce.mean_income = DataLoader::getValue(country, "Avg_Annual_Wage_USD_PPP_2023");
        ce.avg_work_hours_per_year = DataLoader::getValue(country, "Average_Hours_Worked_2023");

        capitalist_economies.push_back(ce);
    }

    /*
        // Print out all economies
        cout << capitalist_economies.size() << " capitalist economies loaded:\n";
        for (const auto &ce : capitalist_economies)
        {
            cout << ce.ce_id << ". " << ce.economy_name
                 << " | GDP Nominal Per Capita: " << ce.gdp_nominal_per_capita
                 << " | GDP PPP Per Capita: " << ce.gdp_ppp_per_capita
                 << " | Average Annual Income: " << ce.mean_income
                 << " | Average Hours Worked: " << ce.avg_work_hours_per_year
                 << "\n";
        }
    */

    // Define products for export and import
    ProductExport solar_panel = {1, "Solar Panel", 20}; // 20 labor hours
    ProductImport laptop_usd = {1, "Laptop", 2000};     // 2000 usd 2023

    // Creating LTE Export Vectors
    vector<LTEExportGDP_Nominal> exports_gdp_nominal; // usd 2023
    vector<LTEExportGDP_PPP> exports_gdp_ppp;         // international usd 2023
    vector<LTEExportMean> exports_mean_income;        // international usd 2023

    // Creating LTE Import Vectors
    vector<LTEImportGDP_Nominal> imports_gdp_nominal; // usd 2023
    vector<LTEImportGDP_PPP> imports_gdp_ppp;         // international usd 2023
    vector<LTEImportMean> imports_mean_income;        // international usd 2023

    // Populate exports_gdp_nominal
    int export_id_gdp_nominal = 1;
    int lte_id = 1; // Single LTE in trade

    for (const auto &ce : capitalist_economies)
    {
        LTEExportGDP_Nominal export_record(
            export_id_gdp_nominal++, // export_id
            lte_id,                  // lte_id
            solar_panel,             // product
            ce                       // capitalist economy
        );

        exports_gdp_nominal.push_back(export_record);
    }

    // Populate exports_gdp_ppp
    int export_id_gdp_ppp = 1;

    for (const auto &ce : capitalist_economies)
    {
        LTEExportGDP_PPP export_record(
            export_id_gdp_ppp++, // export_id
            lte_id,              // lte_id
            solar_panel,         // product
            ce                   // capitalist economy
        );

        exports_gdp_ppp.push_back(export_record);
    }

    // Populate exports_mean_income
    int export_id_mean = 1;

    for (const auto &ce : capitalist_economies)
    {
        LTEExportMean export_record(
            export_id_mean++, // export_id
            lte_id,           // lte_id
            solar_panel,      // product
            ce                // capitalist economy
        );

        exports_mean_income.push_back(export_record);
    }

    // Populate imports_gdp_nominal
    int import_id_gdp_nominal = 1;

    for (const auto &ce : capitalist_economies)
    {
        LTEImportGDP_Nominal import_record(
            import_id_gdp_nominal++, // import_id
            laptop_usd,              // product
            ce,                      // capitalist economy
            lte_id);

        imports_gdp_nominal.push_back(import_record);
    }

    // Populate imports_gdp_ppp
    int import_id_gdp_ppp = 1;

    for (const auto &ce : capitalist_economies)
    {
        ProductImport laptop_int_usd = {1, "Laptop", (2000 * ce.gdp_ppp_per_capita) / ce.gdp_nominal_per_capita};
        LTEImportGDP_PPP import_record(
            import_id_gdp_ppp++, // import_id
            laptop_int_usd,      // product
            ce,                  // capitalist economy
            lte_id);

        imports_gdp_ppp.push_back(import_record);
    }

    // Populate imports_mean_income
    int import_id_mean_income = 1;

    for (const auto &ce : capitalist_economies)
    {
        ProductImport laptop_int_usd = {1, "Laptop", (2000 * ce.gdp_ppp_per_capita) / ce.gdp_nominal_per_capita};
        LTEImportMean import_record(
            import_id_mean_income++, // import_id
            laptop_int_usd,          // product
            ce,                      // capitalist economy
            lte_id);

        imports_mean_income.push_back(import_record);
    }

    // Open output CSV file
    std::ofstream outfile("output.csv");
    if (!outfile.is_open())
    {
        std::cerr << "Failed to open CSV file for writing.\n";
        return 1;
    }

    // Write CSV header
    outfile << "ce_id,country_name,"
            << "export_price_gdp_nominal(20 labor hours to usd 2023),export_price_gdp_ppp(20 labor hours to international usd 2023),export_price_mean_income(20 labor hours to international usd 2023),"
            << "import_price_gdp_nominal(2000 usd 2023 to labor hours),import_price_gdp_ppp(2000 usd 2023 to international usd 2023 to labor hours),import_price_mean_income(2000 usd 2023 to international usd 2023 to labor hours)\n";

    // Write data row-by-row
    for (size_t i = 0; i < capitalist_economies.size(); ++i)
    {
        const auto &ce = capitalist_economies[i];
        const auto &exp_nom = exports_gdp_nominal[i];
        const auto &exp_ppp = exports_gdp_ppp[i];
        const auto &exp_mean = exports_mean_income[i];
        const auto &imp_nom = imports_gdp_nominal[i];
        const auto &imp_ppp = imports_gdp_ppp[i];
        const auto &imp_mean = imports_mean_income[i];

        outfile << ce.ce_id << ","
                << ce.economy_name << ","
                << exp_nom.converted_price_gdp_nominal << ","
                << exp_ppp.converted_price_gdp_ppp << ","
                << exp_mean.converted_price_mean << ","
                << imp_nom.converted_labor_time_gdp_nominal << ","
                << imp_ppp.converted_labor_time_gdp_ppp << ","
                << imp_mean.converted_labor_time_mean << "\n";
    }

    outfile.close();
    std::cout << "CSV file 'output.csv' generated successfully.\n";

    return 0;
}
