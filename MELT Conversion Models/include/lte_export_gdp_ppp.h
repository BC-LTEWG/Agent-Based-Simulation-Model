#pragma once
#include "product_export.h"
#include "capitalist_economy.h"
#include "values.h"

struct LTEExportGDP_PPP
{
    int export_id;
    int lte_id;
    int product_id;
    int ce_id;

    double converted_price_gdp_ppp = INVALID_VALUE;

    // Auto constructor
    LTEExportGDP_PPP(int export_id, int lte_id, const ProductExport &product, const CapitalistEconomy &ce);

    // Manual setter
    void set_value(double new_price);
};
