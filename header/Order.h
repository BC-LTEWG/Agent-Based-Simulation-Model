#pragma once

struct Order {
    enum OrderStatus {
        kOrderRequested,
        kOrderRejected,
        kOrderInProgress,
        kOrderFinished
    };
    Product * product;
    int quantity;
    Firm * customer;
    double requested_resupply_rate;
	double predicted_turnaround_time;
    OrderStatus status;

    Order(
            Product * product,
            int quantity,
            Firm * customer,
            double requested_resupply_rate
         );
};


