#include "Firm.h"
#include "Order.h"
#include "Product.h"

Order::Order(
        Product * product,
        int quantity,
        Firm * customer,
        double requested_turnaround_time
        )
    : product(product),
      quantity(quantity),
      customer(customer),
      requested_turnaround_time(requested_turnaround_time),
      predicted_turnaround_time(requested_turnaround_time),
      status(OrderStatus::kOrderRequested)
{}


