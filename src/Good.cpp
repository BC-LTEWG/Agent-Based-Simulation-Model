#include "Constants.h"
#include "Good.h"

Good::Good() : Product() {
    product_type = Product::ProductType::TYPE_GOOD;
}

void Good::set_corresponding_consumer_good(ConsumerGood * consumer_good) {
    corresponding_consumer_good = consumer_good;
}
