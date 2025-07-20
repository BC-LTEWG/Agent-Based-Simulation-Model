#include "worker.h"

void Worker::pay(double hours) {
    this->hours += hours;
}

void Worker::deduct(double hours) {
    // How should we handle negative balances?
    this->hours -= hours;
}

