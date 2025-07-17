#include "../include/Distributor.hpp"

Distributor::Distributor(Society * society) : Firm(society) {}

void Distributor::add_stock(Good * good, double amount) {
    if (!inventory.count(good)) {
        inventory[good] = 0;
    }
    inventory[good] += amount;
}

std::pair<double, double> Distributor::purchase(Good * good, double amount) {
    if (!inventory.count(good) || inventory[good] <= 0) {
        return {0, 0};
    }

    double available_amount = inventory[good];
    double purchase_amount = std::min(available_amount, amount);
    double cost = purchase_amount * good->value;

    if (purchase_amount > 0) {
        inventory[good] -= purchase_amount;
        return {purchase_amount, cost};
    }
    return {0, 0};
}

void Distributor::display_inventory(int rows) {
    int count = 0;
    for (const auto & [good, amount] : inventory) {
        if (rows > 0 && count >= rows) break;
        printf("%s: %.2f\n", good->name.c_str(), amount);
        count++;
    }
}

void Distributor::head() { display_inventory(5); }