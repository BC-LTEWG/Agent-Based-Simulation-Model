#include "lte_import_gdp_nominal.h"

LTEImportGDP_Nominal::LTEImportGDP_Nominal(int import_id,
                                           const ProductImport &product,
                                           const CapitalistEconomy &ce,
                                           int lte_id)
    : import_id(import_id),
      ce_id(ce.ce_id),
      product_id(product.product_id),
      lte_id(lte_id)
{
    if (product.currency_value > 0 &&
        ce.gdp_nominal_per_capita != INVALID_VALUE &&
        ce.avg_work_hours_per_year > 0)
    {
        converted_labor_time_gdp_nominal =
            (product.currency_value * ce.avg_work_hours_per_year) / ce.gdp_nominal_per_capita;
    }
    else
    {
        converted_labor_time_gdp_nominal = INVALID_VALUE;
    }
}

void LTEImportGDP_Nominal::set_value(double time_hours)
{
    converted_labor_time_gdp_nominal = (time_hours < 0) ? INVALID_VALUE : time_hours;
}
