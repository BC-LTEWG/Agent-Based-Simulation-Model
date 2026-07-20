#include "Firm.h"
#include "Order.h"
#include "Product.h"

Order::Order(
        Product * product,
        int quantity,
        Firm * customer,
        double requested_resupply_rate
        )
    : product(product),
      quantity(quantity),
      customer(customer),
      requested_resupply_rate(requested_resupply_rate),
      status(OrderStatus::kOrderRequested)
{}


