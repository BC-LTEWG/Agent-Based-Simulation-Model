# `Producer`
Producers are a type of `Firm` that produce goods, transforming input products into output products. Mechanisms specific only to producers are described below.

## Initialization
Because the initial inventory needs to last at least `FIRM_STOCKPILE_DURATION + FIRM_DEMAND_WINDOW_MIN` time steps, the producer uses the solution to the ideal Leontief system described by every product's `living_labor_per_unit` and `inputs`, to calculate the required societal production of each of its catalog's products, and scales its initial inventories from this amount. Note that if multiple producers produce the same product, the inventory will likely last much longer.

## Plan Drafting 
A plan is a structure that organizes the fulfillment of an order that the producer recieves. If it recieves an order for which it immediately doesn't have the input products to fulfill it, it rejects the order.

If not, it predicts the amount of workers needed to complete the plan within the `order->deadline`, and starts the draft allocation process. Note that no workers are actually moved until the draft plan is officially pursued by the requesting firm.
### In-Firm Assignment
First, it looks at local workers that are currently on standby. In this process, it prioritizes workers with the least `busyness`, to keep labor utilization as balanced as possible.
### Hiring (Worker Allocation)
Next, it looks to workers that are unemployed, and not assigned to any other Firm. There is no preference in this process, and workers are selected at random.
### Transfers (Worker Reallocation)
If it still hasn't met the worker requirements for the plan, it requests worker transfers from other firms. It sends out a request to producers sequentially, and the producers send back a list of workers they are willing to transfer. 

The decision to transfer has to be justified by a significant labor surplus at the trasferring producer. Specifically, the producer's `busyness`, which is just the average of all its workers' `busyness`, must be less than the societal average busyness by `TRANSFER_BUSYNESS_THRESHOLD`. Moreover, the transferring producer cannot transfer so many workers the resulting predicted local `busyness` would exceed this amount either. 

## Plan Initiation
Once the draft plan is accepted and pursued, the plan initates by _immediately_ consuming the inputs (products and machines), as well as moving all workers out of their current pools (this could be the societal unemployed pool, the current Firm's pool, or another Firm's pool, depending on how the worker was allocated) and into the plan.
## Plan Execution
Plan execution only applies to time steps which are in working hours and working days. Each hour, each worker contributes `suitability / product->living_labor_per_unit` to the `quantity_produced`, and is also paid 1 labor hour for the contribution. Once the `quantity_produced` meets the quantity required by the order, plan completion is initiated.
## Plan Completion
Once the plan is completed, the producer sends the output quantity to the requesting firm, which is modeled as being _instantaneous_. The producer also updates the local `recorded_living_labor_per_unit` for the product (which differs from the ideal `living_labor_per_unit` because of variations in worker suitability), to use to make predictions for future plans.

Additionally, all workers are moved to the Firm's standby pool, waiting to be assigned to a new plan.

