#include "lte_export.h"
#include "values.h"

LTEExport::LTEExport(int export_id, int lte_id, int product_id, int ce_id,
                     double price_gdp_nominal, double price_gdp_ppp,
                     double price_median, double price_mean)
    : export_id(export_id), lte_id(lte_id), product_id(product_id), ce_id(ce_id),
      converted_price_gdp_nominal(price_gdp_nominal),
      converted_price_gdp_ppp(price_gdp_ppp),
      converted_price_median(price_median),
      converted_price_mean(price_mean) {}

// === Manual Constructors ===

LTEExport LTEExport::manual_gdp_nominal(int export_id, int lte_id, int product_id, int ce_id, double price)
{
    return LTEExport(export_id, lte_id, product_id, ce_id, price, INVALID_VALUE, INVALID_VALUE, INVALID_VALUE);
}

LTEExport LTEExport::manual_gdp_ppp(int export_id, int lte_id, int product_id, int ce_id, double price)
{
    return LTEExport(export_id, lte_id, product_id, ce_id, INVALID_VALUE, price, INVALID_VALUE, INVALID_VALUE);
}

LTEExport LTEExport::manual_median(int export_id, int lte_id, int product_id, int ce_id, double price)
{
    return LTEExport(export_id, lte_id, product_id, ce_id, INVALID_VALUE, INVALID_VALUE, price, INVALID_VALUE);
}

LTEExport LTEExport::manual_mean(int export_id, int lte_id, int product_id, int ce_id, double price)
{
    return LTEExport(export_id, lte_id, product_id, ce_id, INVALID_VALUE, INVALID_VALUE, INVALID_VALUE, price);
}

// === Auto Constructors ===

LTEExport LTEExport::auto_gdp_nominal(int export_id, int lte_id, const Product &product, const CapitalistEconomy &ce)
{
    double price = product.labor_time_value * (ce.gdp_nominal_per_capita / ce.avg_work_hours_per_year);
    return LTEExport(export_id, lte_id, product.product_id, ce.ce_id, price, INVALID_VALUE, INVALID_VALUE, INVALID_VALUE);
}

LTEExport LTEExport::auto_gdp_ppp(int export_id, int lte_id, const Product &product, const CapitalistEconomy &ce)
{
    double price = product.labor_time_value * (ce.gdp_ppp_per_capita / ce.avg_work_hours_per_year);
    return LTEExport(export_id, lte_id, product.product_id, ce.ce_id, INVALID_VALUE, price, INVALID_VALUE, INVALID_VALUE);
}

LTEExport LTEExport::auto_median(int export_id, int lte_id, const Product &product, const CapitalistEconomy &ce)
{
    double price = product.labor_time_value * (ce.median_income / ce.avg_work_hours_per_year);
    return LTEExport(export_id, lte_id, product.product_id, ce.ce_id, INVALID_VALUE, INVALID_VALUE, price, INVALID_VALUE);
}

LTEExport LTEExport::auto_mean(int export_id, int lte_id, const Product &product, const CapitalistEconomy &ce)
{
    double price = product.labor_time_value * (ce.mean_income / ce.avg_work_hours_per_year);
    return LTEExport(export_id, lte_id, product.product_id, ce.ce_id, INVALID_VALUE, INVALID_VALUE, INVALID_VALUE, price);
}

// === Setters ===

void LTEExport::set_price_gdp_nominal(double new_price)
{
    converted_price_gdp_nominal = new_price;
    converted_price_gdp_ppp = converted_price_median = converted_price_mean = INVALID_VALUE;
}

void LTEExport::set_price_gdp_ppp(double new_price)
{
    converted_price_gdp_ppp = new_price;
    converted_price_gdp_nominal = converted_price_median = converted_price_mean = INVALID_VALUE;
}

void LTEExport::set_price_median(double new_price)
{
    converted_price_median = new_price;
    converted_price_gdp_nominal = converted_price_gdp_ppp = converted_price_mean = INVALID_VALUE;
}

void LTEExport::set_price_mean(double new_price)
{
    converted_price_mean = new_price;
    converted_price_gdp_nominal = converted_price_gdp_ppp = converted_price_median = INVALID_VALUE;
}

// === Mode Checkers ===

bool LTEExport::uses_gdp_nominal() const
{
    return converted_price_gdp_nominal != INVALID_VALUE;
}

bool LTEExport::uses_gdp_ppp() const
{
    return converted_price_gdp_ppp != INVALID_VALUE;
}

bool LTEExport::uses_median() const
{
    return converted_price_median != INVALID_VALUE;
}

bool LTEExport::uses_mean() const
{
    return converted_price_mean != INVALID_VALUE;
}
