# Distributor Agent

## Purpose
The Distributor holds inventory of consumer goods and fulfills purchases from people. It tracks demand signals, sells from stock and triggers restocking when inventory is short.

## Initialization
- Builds an initial inventory for every product in its catalog.
- Registers each product as a `ConsumerGood` in the `Society`.
- Creates an internal `Plan` per product to track labor usage and revenue for distribution activity.

## Per-timestep behavior
- Delegates generic firm logic to `Firm::on_time_step()`.
- Advances each distribution plan by consuming worker labor hours and recording worked hours.

## Selling goods
`sell_goods(Product& product, int quantity, Person * person)`:
- Adds a demand signal for the requested quantity.
- If inventory is short, creates a new order for the remainder and sends it to producers.
- Charges the buyer for the portion actually sold and records revenue on the product's plan.
- Reduces inventory by the quantity sold.

## Reordering
The Distributor reorders only for products in its catalog.
