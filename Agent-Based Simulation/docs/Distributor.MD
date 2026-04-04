# `Distributor`
Distributors are a type of `Firm` that sell goods to consumers. Mechanisms specific only to distributors are described below.

## Initialization
Because the initial inventory needs to last at least `FIRM_STOCKPILE_DURATION + FIRM_DEMAND_WINDOW_MIN` time steps, distributors scale this value off of each of their catalog's product's `mean_consumption_frequency`. Note that if multiple distributors sell the same product, the inventory will likely last much longer.

Additionally, distributors create a `ConsumerGood` (if it doesn't exist) for products they're assigned, which is simply a representation of the original good with added distribution cost. 

## Selling goods
Selling is implemented with `try_sell_goods()`, which people call during their shopping events. In normal circumstances, will successfully exchange the socially necessary labor time for the requested quantity of goods. But, if the distributor doesn't have enough inventory to fulfill the request, it will only sell as much as it can. In either case, however, the demand signal logged for the full requested amount.
