#include "Distributor.h"


double Distributor::getOutputRatio(Product& product) {
    return 1 / product.orderSize;

}

double Distributor::plannedSatisfactionPerPerson(Product& product, Person& person) {
    return getOutputRatio(product) * person.getWorkerNeeds()[product.productName];
}