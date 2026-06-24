#pragma once

struct Order {
    enum OrderStatus { ORDER_REQUESTED, ORDER_REJECTED, ORDER_IN_PROGRESS, ORDER_FINISHED };
    Product * product;
    int quantity;
    Firm * customer;
    double requested_turnaround_time;
    OrderStatus status;
    
    Order(
            Product * product,
            int quantity,
            Firm * customer,
            double requested_turnaround_time
    );
};


