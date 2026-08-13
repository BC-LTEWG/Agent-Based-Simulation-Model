#pragma once

struct Order {
    unsigned int id;
    enum OrderStatus {
        kOrderRequested,
        kOrderRejected,
        kOrderInProgress,
        kOrderFinished
    };
    Product * product;
    int quantity;
    Firm * customer;
    double requested_turnaround_time;
	double predicted_turnaround_time;
    OrderStatus status;

    Order(
            Product * product,
            int quantity,
            Firm * customer,
            double requested_turnaround_time
         );
};


