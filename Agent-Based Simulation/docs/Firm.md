# `Firm`
Firms are a general class that represent an organization of workers that coordinate the flow of goods throughout the society, and are separated into `Producer`s and `Distributor`s. They share many mechanisms, listed below.

## Inventory Management
### Initialization
All firms are created with a catalog of products that they produce or distribute. At the beginning of time, it's important that their inventories are non-empty to avoid immediate shortages and societal collapse. In general, these initial inventories are designed to last `FIRM_STOCKPILE_DURATION + FIRM_DEMAND_WINDOW_MIN` time steps.
### Demand Tracking
Firms track a local demand for their _input products_ via `add_demand_signal()`, updating a demand indexed by product, in units of quantity per time step. This demand is averaged over a window of time between `FIRM_DEMAND_WINDOW_MIN` and `FIRM_DEMAND_WINDOW_MAX`, depending on the usable data available to the firm. For example, a firm will only average demand over the minimum window size if it just has been created, while a firm that has long been in operation will average demand over the maximum window size. 
### Reordering
Firms reorder input products when inventory falls below a threshold that is `threshold = demand[product] * FIRM_STOCKPILE_DURATION`. When this happens, it dispatches an `Order` out to all producers of the relevant product, requesting a quantity of `threshold`, and a deadline of `pending_inventory[product] / demand[product] * DEADLINE_SAFETY_MULT`. Essentially, the firm wants to pre-emptively replenish its inventory to above the threshold before it runs out completely, basing its prediction off of current demand. Its also important to note that the `pending_inventory` includes inventory that has already been ordered but has not yet arrived, so that orders are not redundantly placed.

All contancted producers will send an estimate of their plan's `turnaround_time`, and the firm will pursue the order with the producer with the lowest turnaround time.
