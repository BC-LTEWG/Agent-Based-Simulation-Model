#include "lte_import.h"
#include "values.h"

LTEImport::LTEImport(int import_id, int ce_id, int product_id, int lte_id,
                     double time_gdp_nominal, double time_gdp_ppp,
                     double time_median, double time_mean)
    : import_id(import_id), ce_id(ce_id), product_id(product_id), lte_id(lte_id),
      converted_labor_time_gdp_nominal(time_gdp_nominal),
      converted_labor_time_gdp_ppp(time_gdp_ppp),
      converted_labor_time_median(time_median),
      converted_labor_time_mean(time_mean) {}

// === Manual Constructors ===

LTEImport LTEImport::manual_gdp_nominal(int import_id, int ce_id, int product_id, int lte_id, double time)
{
    return LTEImport(import_id, ce_id, product_id, lte_id, time, INVALID_VALUE, INVALID_VALUE, INVALID_VALUE);
}

LTEImport LTEImport::manual_gdp_ppp(int import_id, int ce_id, int product_id, int lte_id, double time)
{
    return LTEImport(import_id, ce_id, product_id, lte_id, INVALID_VALUE, time, INVALID_VALUE, INVALID_VALUE);
}

LTEImport LTEImport::manual_median(int import_id, int ce_id, int product_id, int lte_id, double time)
{
    return LTEImport(import_id, ce_id, product_id, lte_id, INVALID_VALUE, INVALID_VALUE, time, INVALID_VALUE);
}

LTEImport LTEImport::manual_mean(int import_id, int ce_id, int product_id, int lte_id, double time)
{
    return LTEImport(import_id, ce_id, product_id, lte_id, INVALID_VALUE, INVALID_VALUE, INVALID_VALUE, time);
}

// === Auto Constructors ===

LTEImport LTEImport::auto_gdp_nominal(int import_id, const Product &product, const CapitalistEconomy &ce, int lte_id)
{
    double time = product.currency_value * (ce.avg_work_hours_per_year / ce.gdp_nominal_per_capita);
    return LTEImport(import_id, ce.ce_id, product.product_id, lte_id, time, INVALID_VALUE, INVALID_VALUE, INVALID_VALUE);
}

LTEImport LTEImport::auto_gdp_ppp(int import_id, const Product &product, const CapitalistEconomy &ce, int lte_id)
{
    double time = product.currency_value * (ce.avg_work_hours_per_year / ce.gdp_ppp_per_capita);
    return LTEImport(import_id, ce.ce_id, product.product_id, lte_id, INVALID_VALUE, time, INVALID_VALUE, INVALID_VALUE);
}

LTEImport LTEImport::auto_median(int import_id, const Product &product, const CapitalistEconomy &ce, int lte_id)
{
    double time = product.currency_value * (ce.avg_work_hours_per_year / ce.median_income);
    return LTEImport(import_id, ce.ce_id, product.product_id, lte_id, INVALID_VALUE, INVALID_VALUE, time, INVALID_VALUE);
}

LTEImport LTEImport::auto_mean(int import_id, const Product &product, const CapitalistEconomy &ce, int lte_id)
{
    double time = product.currency_value * (ce.avg_work_hours_per_year / ce.mean_income);
    return LTEImport(import_id, ce.ce_id, product.product_id, lte_id, INVALID_VALUE, INVALID_VALUE, INVALID_VALUE, time);
}

// === Setters ===

void LTEImport::set_time_gdp_nominal(double time)
{
    converted_labor_time_gdp_nominal = time;
    converted_labor_time_gdp_ppp = converted_labor_time_median = converted_labor_time_mean = INVALID_VALUE;
}

void LTEImport::set_time_gdp_ppp(double time)
{
    converted_labor_time_gdp_ppp = time;
    converted_labor_time_gdp_nominal = converted_labor_time_median = converted_labor_time_mean = INVALID_VALUE;
}

void LTEImport::set_time_median(double time)
{
    converted_labor_time_median = time;
    converted_labor_time_gdp_nominal = converted_labor_time_gdp_ppp = converted_labor_time_mean = INVALID_VALUE;
}

void LTEImport::set_time_mean(double time)
{
    converted_labor_time_mean = time;
    converted_labor_time_gdp_nominal = converted_labor_time_gdp_ppp = converted_labor_time_median = INVALID_VALUE;
}

// === Checkers ===

bool LTEImport::uses_gdp_nominal() const
{
    return converted_labor_time_gdp_nominal != INVALID_VALUE;
}

bool LTEImport::uses_gdp_ppp() const
{
    return converted_labor_time_gdp_ppp != INVALID_VALUE;
}

bool LTEImport::uses_median() const
{
    return converted_labor_time_median != INVALID_VALUE;
}

bool LTEImport::uses_mean() const
{
    return converted_labor_time_mean != INVALID_VALUE;
}
