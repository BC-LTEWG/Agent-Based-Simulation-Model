#pragma once
#include "product.h"
#include "capitalist_economy.h"
#include "labortime_economy.h"

class LTEImport
{
public:
    int import_id;
    int ce_id;
    int product_id;
    int lte_id;

    double converted_labor_time_gdp_nominal;
    double converted_labor_time_gdp_ppp;
    double converted_labor_time_median;
    double converted_labor_time_mean;

    // Manual constructors
    static LTEImport manual_gdp_nominal(int import_id, int ce_id, int product_id, int lte_id, double time);
    static LTEImport manual_gdp_ppp(int import_id, int ce_id, int product_id, int lte_id, double time);
    static LTEImport manual_median(int import_id, int ce_id, int product_id, int lte_id, double time);
    static LTEImport manual_mean(int import_id, int ce_id, int product_id, int lte_id, double time);

    // Auto constructors
    static LTEImport auto_gdp_nominal(int import_id, const Product &product, const CapitalistEconomy &ce, int lte_id);
    static LTEImport auto_gdp_ppp(int import_id, const Product &product, const CapitalistEconomy &ce, int lte_id);
    static LTEImport auto_median(int import_id, const Product &product, const CapitalistEconomy &ce, int lte_id);
    static LTEImport auto_mean(int import_id, const Product &product, const CapitalistEconomy &ce, int lte_id);

    // Setters
    void set_time_gdp_nominal(double time);
    void set_time_gdp_ppp(double time);
    void set_time_median(double time);
    void set_time_mean(double time);

    // Active mode checkers
    bool uses_gdp_nominal() const;
    bool uses_gdp_ppp() const;
    bool uses_median() const;
    bool uses_mean() const;

private:
    LTEImport(int import_id, int ce_id, int product_id, int lte_id,
              double time_gdp_nominal, double time_gdp_ppp,
              double time_median, double time_mean);
};
