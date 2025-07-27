#include "lte_export_median.h"

LTEExportMedian::LTEExportMedian(int export_id, int lte_id, const ProductExport &product, const CapitalistEconomy &ce)
    : export_id(export_id), lte_id(lte_id), product_id(product.product_id), ce_id(ce.ce_id)
{
    if (product.labor_time_value > 0 && ce.median_income != INVALID_VALUE && ce.avg_work_hours_per_year > 0)
    {
        converted_price_median = product.labor_time_value * (ce.median_income / ce.avg_work_hours_per_year);
    }
    else
    {
        converted_price_median = INVALID_VALUE;
    }
}

void LTEExportMedian::set_value(double new_price)
{
    converted_price_median = (new_price < 0) ? INVALID_VALUE : new_price;
}
