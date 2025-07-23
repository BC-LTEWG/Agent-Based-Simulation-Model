#pragma once
#include "product.h"
#include "labortime_economy.h"
#include "capitalist_economy.h"

class LTEExport
{
public:
    int export_id;
    int lte_id;
    int product_id;
    int ce_id;

    double converted_price_gdp_nominal;
    double converted_price_gdp_ppp;
    double converted_price_median;
    double converted_price_mean;

    // Static manual constructors
    static LTEExport manual_gdp_nominal(int export_id, int lte_id, int product_id, int ce_id, double price);
    static LTEExport manual_gdp_ppp(int export_id, int lte_id, int product_id, int ce_id, double price);
    static LTEExport manual_median(int export_id, int lte_id, int product_id, int ce_id, double price);
    static LTEExport manual_mean(int export_id, int lte_id, int product_id, int ce_id, double price);

    // Static automatic constructors
    static LTEExport auto_gdp_nominal(int export_id, int lte_id, const Product &product, const CapitalistEconomy &ce);
    static LTEExport auto_gdp_ppp(int export_id, int lte_id, const Product &product, const CapitalistEconomy &ce);
    static LTEExport auto_median(int export_id, int lte_id, const Product &product, const CapitalistEconomy &ce);
    static LTEExport auto_mean(int export_id, int lte_id, const Product &product, const CapitalistEconomy &ce);

    // Setters
    void set_price_gdp_nominal(double new_price);
    void set_price_gdp_ppp(double new_price);
    void set_price_median(double new_price);
    void set_price_mean(double new_price);

    // Mode checkers
    bool uses_gdp_nominal() const;
    bool uses_gdp_ppp() const;
    bool uses_median() const;
    bool uses_mean() const;

private:
    LTEExport(int export_id, int lte_id, int product_id, int ce_id,
              double price_gdp_nominal, double price_gdp_ppp,
              double price_median, double price_mean);
};
