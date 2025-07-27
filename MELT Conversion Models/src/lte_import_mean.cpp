#include "lte_import_mean.h"

LTEImportMean::LTEImportMean(int import_id,
                             const ProductImport &product,
                             const CapitalistEconomy &ce,
                             int lte_id)
    : import_id(import_id),
      ce_id(ce.ce_id),
      product_id(product.product_id),
      lte_id(lte_id)
{
    if (product.currency_value > 0 &&
        ce.mean_income != INVALID_VALUE &&
        ce.avg_work_hours_per_year > 0)
    {
        converted_labor_time_mean =
            (product.currency_value * ce.avg_work_hours_per_year) / ce.mean_income;
    }
    else
    {
        converted_labor_time_mean = INVALID_VALUE;
    }
}

void LTEImportMean::set_value(double time_hours)
{
    converted_labor_time_mean = (time_hours < 0) ? INVALID_VALUE : time_hours;
}
