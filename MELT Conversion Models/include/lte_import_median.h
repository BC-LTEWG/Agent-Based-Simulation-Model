#pragma once
#include "product_import.h"
#include "capitalist_economy.h"
#include "values.h"

struct LTEImportMedian
{
    int import_id;
    int ce_id;
    int product_id;
    int lte_id;

    double converted_labor_time_median = INVALID_VALUE;

    // Auto constructor
    LTEImportMedian(int import_id, const ProductImport &product,
                    const CapitalistEconomy &ce, int lte_id);

    // Manual setter
    void set_value(double time_hours);
};
