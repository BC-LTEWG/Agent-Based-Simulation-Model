#include "lte_export_gdp_ppp.h"
#include <cmath>

LTEExportGDP_PPP::LTEExportGDP_PPP(int export_id, int lte_id, const ProductExport &product, const CapitalistEconomy &ce)
    : export_id(export_id), lte_id(lte_id), product_id(product.product_id), ce_id(ce.ce_id)
{
    if (product.labor_time_value > 0 && ce.gdp_ppp_per_capita != INVALID_VALUE && ce.avg_work_hours_per_year > 0)
    {
        converted_price_gdp_ppp = product.labor_time_value * (ce.gdp_ppp_per_capita / ce.avg_work_hours_per_year);
    }
    else
    {
        converted_price_gdp_ppp = INVALID_VALUE;
    }
}

void LTEExportGDP_PPP::set_value(double new_price)
{
    converted_price_gdp_ppp = (new_price < 0) ? INVALID_VALUE : new_price;
}
