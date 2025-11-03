#include "Distributor.h"

Distributor::Distributor() : Firm() {}

Distributor::Distributor(std::vector<Machine*> machines, std::vector<Person*> workforce, std::vector<Plan*> plans) 
    : Firm(machines, workforce, plans) {}

double Distributor::get_output_ratio(Product& product) {
    return 1.0 / product.order_size;
}

double Distributor::planned_satisfaction_per_person(Product& product, Person& person) {
    return get_output_ratio(product) * person.get_worker_needs()[product.product_name];
}
