#include <iostream>
#include "values.h"
#include "product.h"
#include "capitalist_economy.h"
#include "labortime_economy.h"
#include "lte_export.h"
#include "lte_import.h"

using namespace std;

/*
Canada, France, Germany, Japan
*/

int main()
{
    // Step 1: Define a Labor-Time Economy
    LaborTimeEconomy lte(1, "LTE");

    // Step 2: Define Capitalist Economies (2023)
    // All currencies are converted to us dollars
    CapitalistEconomy Canada_gdp_nominal = CapitalistEconomy::from_gdp_nominal(1, "Canada", CANADA_GDP_NOMINAL, CANADA_AVG_HOURS);
    CapitalistEconomy France_gdp_nominal = CapitalistEconomy::from_gdp_nominal(2, "France", FRANCE_GDP_NOMINAL, FRANCE_AVG_HOURS);
    CapitalistEconomy Germany_gdp_nominal = CapitalistEconomy::from_gdp_nominal(3, "Germany", GERMANY_GDP_NOMINAL, GERMANY_AVG_HOURS);
    CapitalistEconomy Japan_gdp_nominal = CapitalistEconomy::from_gdp_nominal(4, "Japan", JAPAN_GDP_NOMINAL, JAPAN_AVG_HOURS);

    CapitalistEconomy Canada_gdp_ppp = CapitalistEconomy::from_gdp_ppp(1, "Canada", CANADA_GDP_PPP, CANADA_AVG_HOURS);
    CapitalistEconomy France_gdp_ppp = CapitalistEconomy::from_gdp_ppp(2, "France", FRANCE_GDP_PPP, FRANCE_AVG_HOURS);
    CapitalistEconomy Germany_gdp_ppp = CapitalistEconomy::from_gdp_ppp(3, "Germany", GERMANY_GDP_PPP, GERMANY_AVG_HOURS);
    CapitalistEconomy Japan_gdp_ppp = CapitalistEconomy::from_gdp_ppp(4, "Japan", JAPAN_GDP_PPP, JAPAN_AVG_HOURS);

    CapitalistEconomy Canada_median_income = CapitalistEconomy::from_median_income(1, "Canada", CANADA_MEDIAN_INCOME, CANADA_AVG_HOURS);
    CapitalistEconomy France_median_income = CapitalistEconomy::from_median_income(2, "France", FRANCE_MEDIAN_INCOME, FRANCE_AVG_HOURS);
    CapitalistEconomy Germany_median_income = CapitalistEconomy::from_median_income(3, "Germany", GERMANY_MEDIAN_INCOME, GERMANY_AVG_HOURS);
    CapitalistEconomy Japan_median_income = CapitalistEconomy::from_median_income(4, "Japan", JAPAN_MEDIAN_INCOME, JAPAN_AVG_HOURS);

    CapitalistEconomy Canada_mean_income = CapitalistEconomy::from_mean_income(1, "Canada", CANADA_MEAN_INCOME, CANADA_AVG_HOURS);
    CapitalistEconomy France_mean_income = CapitalistEconomy::from_mean_income(2, "France", FRANCE_MEAN_INCOME, FRANCE_AVG_HOURS);
    CapitalistEconomy Germany_mean_income = CapitalistEconomy::from_mean_income(3, "Germany", GERMANY_MEAN_INCOME, GERMANY_AVG_HOURS);
    CapitalistEconomy Japan_mean_income = CapitalistEconomy::from_mean_income(4, "Japan", JAPAN_MEAN_INCOME, JAPAN_AVG_HOURS);

    // Step 3: Define the products for export and import
    Product solar_panel_export = Product::for_export(1, "Laptop", 20); // 20 labor hours
    Product laptop_import = Product::for_import(2, "Laptop", 1200);    // $1200

    // Step 4: Convert labor-time value to money for export
    LTEExport export_to_canada_gdp_nominal = LTEExport::auto_gdp_nominal(1001, lte.lte_id, solar_panel_export, Canada_gdp_nominal);
    LTEExport export_to_france_gdp_nominal = LTEExport::auto_gdp_nominal(1002, lte.lte_id, solar_panel_export, France_gdp_nominal);
    LTEExport export_to_germany_gdp_nominal = LTEExport::auto_gdp_nominal(1003, lte.lte_id, solar_panel_export, Germany_gdp_nominal);
    LTEExport export_to_japan_gdp_nominal = LTEExport::auto_gdp_nominal(1004, lte.lte_id, solar_panel_export, Japan_gdp_nominal);

    LTEExport export_to_canada_gdp_ppp = LTEExport::auto_gdp_ppp(1005, lte.lte_id, solar_panel_export, Canada_gdp_ppp);
    LTEExport export_to_france_gdp_ppp = LTEExport::auto_gdp_ppp(1006, lte.lte_id, solar_panel_export, France_gdp_ppp);
    LTEExport export_to_germany_gdp_ppp = LTEExport::auto_gdp_ppp(1007, lte.lte_id, solar_panel_export, Germany_gdp_ppp);
    LTEExport export_to_japan_gdp_ppp = LTEExport::auto_gdp_ppp(1008, lte.lte_id, solar_panel_export, Japan_gdp_ppp);

    LTEExport export_to_canada_median = LTEExport::auto_median(1009, lte.lte_id, solar_panel_export, Canada_median_income);
    LTEExport export_to_france_median = LTEExport::auto_median(1010, lte.lte_id, solar_panel_export, France_median_income);
    LTEExport export_to_germany_median = LTEExport::auto_median(1011, lte.lte_id, solar_panel_export, Germany_median_income);
    LTEExport export_to_japan_median = LTEExport::auto_median(1012, lte.lte_id, solar_panel_export, Japan_median_income);

    LTEExport export_to_canada_mean = LTEExport::auto_mean(1013, lte.lte_id, solar_panel_export, Canada_mean_income);
    LTEExport export_to_france_mean = LTEExport::auto_mean(1014, lte.lte_id, solar_panel_export, France_mean_income);
    LTEExport export_to_germany_mean = LTEExport::auto_mean(1015, lte.lte_id, solar_panel_export, Germany_mean_income);
    LTEExport export_to_japan_mean = LTEExport::auto_mean(1016, lte.lte_id, solar_panel_export, Japan_mean_income);

    // Step 5: Convert money to labor-time for import
    LTEImport import_from_canada_gdp_nominal = LTEImport::auto_gdp_nominal(2001, laptop_import, Canada_gdp_nominal, lte.lte_id);
    LTEImport import_from_france_gdp_nominal = LTEImport::auto_gdp_nominal(2002, laptop_import, France_gdp_nominal, lte.lte_id);
    LTEImport import_from_germany_gdp_nominal = LTEImport::auto_gdp_nominal(2003, laptop_import, Germany_gdp_nominal, lte.lte_id);
    LTEImport import_from_japan_gdp_nominal = LTEImport::auto_gdp_nominal(2004, laptop_import, Japan_gdp_nominal, lte.lte_id);

    LTEImport import_from_canada_gdp_ppp = LTEImport::auto_gdp_ppp(2005, laptop_import, Canada_gdp_ppp, lte.lte_id);
    LTEImport import_from_france_gdp_ppp = LTEImport::auto_gdp_ppp(2006, laptop_import, France_gdp_ppp, lte.lte_id);
    LTEImport import_from_germany_gdp_ppp = LTEImport::auto_gdp_ppp(2007, laptop_import, Germany_gdp_ppp, lte.lte_id);
    LTEImport import_from_japan_gdp_ppp = LTEImport::auto_gdp_ppp(2008, laptop_import, Japan_gdp_ppp, lte.lte_id);

    LTEImport import_from_canada_median = LTEImport::auto_median(2009, laptop_import, Canada_median_income, lte.lte_id);
    LTEImport import_from_france_median = LTEImport::auto_median(2010, laptop_import, France_median_income, lte.lte_id);
    LTEImport import_from_germany_median = LTEImport::auto_median(2011, laptop_import, Germany_median_income, lte.lte_id);
    LTEImport import_from_japan_median = LTEImport::auto_median(2012, laptop_import, Japan_median_income, lte.lte_id);

    LTEImport import_from_canada_mean = LTEImport::auto_mean(2013, laptop_import, Canada_mean_income, lte.lte_id);
    LTEImport import_from_france_mean = LTEImport::auto_mean(2014, laptop_import, France_mean_income, lte.lte_id);
    LTEImport import_from_germany_mean = LTEImport::auto_mean(2015, laptop_import, Germany_mean_income, lte.lte_id);
    LTEImport import_from_japan_mean = LTEImport::auto_mean(2016, laptop_import, Japan_mean_income, lte.lte_id);

    // Step 6: Print Results
    // all converted to us dollars
    cout << "Export using nominal GDP per capita" << "\n";
    cout << "\n";

    cout << "Exported solar panel (20 labor hours) to Canada using nominal GDP per capita → $" << export_to_canada_gdp_nominal.converted_price_gdp_nominal << "\n";
    cout << "Exported solar panel (20 labor hours) to France using nominal GDP per capita → $" << export_to_france_gdp_nominal.converted_price_gdp_nominal << "\n";
    cout << "Exported solar panel (20 labor hours) to Germany using nominal GDP per capita → $" << export_to_germany_gdp_nominal.converted_price_gdp_nominal << "\n";
    cout << "Exported solar panel (20 labor hours) to Japan using nominal GDP per capita → $" << export_to_japan_gdp_nominal.converted_price_gdp_nominal << "\n";

    cout << "\n";
    cout << "Export using GDP PPP per capita" << "\n";
    cout << "\n";

    cout << "Exported solar panel (20 labor hours) to Canada using GDP PPP per capita → $" << export_to_canada_gdp_ppp.converted_price_gdp_ppp << "\n";
    cout << "Exported solar panel (20 labor hours) to France using GDP PPP per capita → $" << export_to_france_gdp_ppp.converted_price_gdp_ppp << "\n";
    cout << "Exported solar panel (20 labor hours) to Germany using GDP PPP per capita → $" << export_to_germany_gdp_ppp.converted_price_gdp_ppp << "\n";
    cout << "Exported solar panel (20 labor hours) to Japan using GDP PPP per capita → $" << export_to_japan_gdp_ppp.converted_price_gdp_ppp << "\n";

    cout << "\n";
    cout << "Export using median income" << "\n";
    cout << "\n";

    cout << "Exported solar panel (20 labor hours) to Canada using median income → $" << export_to_canada_median.converted_price_median << "\n";
    cout << "Exported solar panel (20 labor hours) to France using median income → $" << export_to_france_median.converted_price_median << "\n";
    cout << "Exported solar panel (20 labor hours) to Germany using median income → $" << export_to_germany_median.converted_price_median << "\n";
    cout << "Exported solar panel (20 labor hours) to Japan using median income → $" << export_to_japan_median.converted_price_median << "\n";

    cout << "\n";
    cout << "Export using mean income" << "\n";
    cout << "\n";

    cout << "Exported solar panel (20 labor hours) to Canada using mean income → $" << export_to_canada_mean.converted_price_mean << "\n";
    cout << "Exported solar panel (20 labor hours) to France using mean income → $" << export_to_france_mean.converted_price_mean << "\n";
    cout << "Exported solar panel (20 labor hours) to Germany using mean income → $" << export_to_germany_mean.converted_price_mean << "\n";
    cout << "Exported solar panel (20 labor hours) to Japan using mean income → $" << export_to_japan_mean.converted_price_mean << "\n";

    cout << "\n";
    cout << "Import using nominal GDP per capita" << "\n";
    cout << "\n";

    cout << "Imported laptop ($1200) from Canada using nominal GDP per capita → " << import_from_canada_gdp_nominal.converted_labor_time_gdp_nominal << " labor hours\n";
    cout << "Imported laptop ($1200) from France using nominal GDP per capita → " << import_from_france_gdp_nominal.converted_labor_time_gdp_nominal << " labor hours\n";
    cout << "Imported laptop ($1200) from Germany using nominal GDP per capita → " << import_from_germany_gdp_nominal.converted_labor_time_gdp_nominal << " labor hours\n";
    cout << "Imported laptop ($1200) from Japan using nominal GDP per capita → " << import_from_japan_gdp_nominal.converted_labor_time_gdp_nominal << " labor hours\n";

    cout << "\n";
    cout << "Import using GDP PPP per capita" << "\n";
    cout << "\n";

    cout << "Imported laptop ($1200) from Canada using GDP PPP per capita → " << import_from_canada_gdp_ppp.converted_labor_time_gdp_ppp << " labor hours\n";
    cout << "Imported laptop ($1200) from France using GDP PPP per capita → " << import_from_france_gdp_ppp.converted_labor_time_gdp_ppp << " labor hours\n";
    cout << "Imported laptop ($1200) from Germany using GDP PPP per capita → " << import_from_germany_gdp_ppp.converted_labor_time_gdp_ppp << " labor hours\n";
    cout << "Imported laptop ($1200) from Japan using GDP PPP per capita → " << import_from_japan_gdp_ppp.converted_labor_time_gdp_ppp << " labor hours\n";

    cout << "\n";
    cout << "Import using median income" << "\n";
    cout << "\n";

    cout << "Imported laptop ($1200) from Canada using median income → " << import_from_canada_median.converted_labor_time_median << " labor hours\n";
    cout << "Imported laptop ($1200) from France using median income → " << import_from_france_median.converted_labor_time_median << " labor hours\n";
    cout << "Imported laptop ($1200) from Germany using median income → " << import_from_germany_median.converted_labor_time_median << " labor hours\n";
    cout << "Imported laptop ($1200) from Japan using median income → " << import_from_japan_median.converted_labor_time_median << " labor hours\n";

    cout << "\n";
    cout << "Import using mean income" << "\n";
    cout << "\n";

    cout << "Imported laptop ($1200) from Canada using mean income → " << import_from_canada_mean.converted_labor_time_mean << " labor hours\n";
    cout << "Imported laptop ($1200) from France using mean income → " << import_from_france_mean.converted_labor_time_mean << " labor hours\n";
    cout << "Imported laptop ($1200) from Germany using mean income → " << import_from_germany_mean.converted_labor_time_mean << " labor hours\n";
    cout << "Imported laptop ($1200) from Japan using mean income → " << import_from_japan_mean.converted_labor_time_mean << " labor hours\n";

    return 0;
}
