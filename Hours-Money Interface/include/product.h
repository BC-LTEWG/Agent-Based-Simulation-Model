#pragma once
#include <string>

class Product
{
public:
    int product_id;
    std::string product_name;
    double labor_time_value; // Used in exports (from LTE to CE)
    double currency_value;   // Used in imports (from CE to LTE)

    // Named constructors
    static Product for_export(int id, const std::string &name, double labor_hours);
    static Product for_import(int id, const std::string &name, double currency);

    // Type checkers
    bool is_export() const;
    bool is_import() const;

private:
    Product(int id, const std::string &name, double ltv, double cv);
};
