#include <string>

#include "Machine.h"

Machine::Machine(int id, const std::string& name, int lifetime) :
    Product{id, name},
    lifetime{lifetime}
{
    product_type = Product::ProductType::TYPE_MACHINE;
    mean_consumption_frequency = 0;
}

