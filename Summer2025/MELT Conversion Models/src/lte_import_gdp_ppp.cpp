#include "lte_import_gdp_ppp.h"

LTEImportGDP_PPP::LTEImportGDP_PPP(int import_id,
                                   const ProductImport &product,
                                   const CapitalistEconomy &ce,
                                   int lte_id)
    : import_id(import_id),
      ce_id(ce.ce_id),
      product_id(product.product_id),
      lte_id(lte_id)
{
    if (product.currency_value > 0 &&
        ce.gdp_ppp_per_capita != INVALID_VALUE &&
        ce.avg_work_hours_per_year > 0)
    {
        converted_labor_time_gdp_ppp =
            (product.currency_value * ce.avg_work_hours_per_year) / ce.gdp_ppp_per_capita;
    }
    else
    {
        converted_labor_time_gdp_ppp = INVALID_VALUE;
    }
}

void LTEImportGDP_PPP::set_value(double time_hours)
{
    converted_labor_time_gdp_ppp = (time_hours < 0) ? INVALID_VALUE : time_hours;
}
