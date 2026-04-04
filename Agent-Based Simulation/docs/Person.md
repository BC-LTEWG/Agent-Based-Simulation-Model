# `Person`
People are the smallest agents in the society, and have three primary mechanisms, listed below.

## Aging and Retirement
People are not instantiated at the moment they are born, but rather at a constant `INITIAL_AGE`, in time steps of hours. We assume that, before this point, they neither consume labor hours (buying goods) nor produce them (work). 
  - For example, if `INITIAL_AGE = 157680`, they effectively ``enter'' the society at 18 years of age.

Since, at the moment, we model society as that portion of society involved in productive work, retiring consists of a person being removed from the society. (This will change when we extend society by implementing public sector services such as retirement income.) On every time step after instantiation, Persons have a probability of `RANDOM_RETIREMENT_CHANCE` to retire before `GUARANTEED_RETIREMENT_AGE`. On attaining `GUARANTEED_RETIREMENT_AGE`, they automatically retire, if they have not already done so. 
  - For example, if `RANDOM_RETIREMENT_CHANCE = 8.3e-7` and `GUARANTEED_RETIREMENT_AGE = 586920`, all persons are guaranteed to retire before 67 years, with the average being around 62 years of age.

## Shopping and Consumption
The consumption of products by people is the primary "pull" mechanism for production in the simulation. The rate of consumption is determined by each each `Product`'s `mean_consumption_frequency`, which describes the number of units, on average, of a product is consumed per time step, or removed from a person's inventory every time step.

Every person aims to keep an inventory of at least `PERSON_STOCKPILE_DURATION * mean_consumption_frequency` for each good. If at any point, the combined _time_ deficit under `PERSON_STOCKPILE_DURATION` is at least `PERSON_DEFICIT_THRESHOLD`, a person will decide that they must initiate a _shopping event_. In this event, they simply purchase enough of each product to bring their inventories back to the ideal stockpile level.

To purchase each good, a person searches sequentially through their list of `ranked_distributors` (a random permutation for each person), until they find a `Distributor` with the item in stock. If the distributor doesn't meet their entire demand, they keep going through the list. Additionally, if a person doesn't have enough labor hours in their `account` to purchase what they need, they scale their "shopping list" down proportionally until they are able to.

## Work and Busyness
A person's rate of work is determined by the product being produced (specifically, its `required_abilities`), their own `abilities`, and their `health_status`. 

Each of a person's abilities is a `double` representing their skill level in that line of work, with `1.0` representing an average work rate. These values are determined for each person at construction by constants `PERSON_ABILITY_COUNT_MAX` and `PERSON_ABILITY_STDDEV`. For each of a product's `required_abilities`, a person's ability values are averaged, then multiplied by `get_current_productivity()`, to help a firm to decide how suitable that person is to help produce that product. `get_current_productivity()` is meant to represent short term variations in a person's work rate, and for now only depends on whether a person is `HEALTHY` or `UNHEALTHY`.

A person does not work all the time, even when assigned to a `Plan`, as they are restricted by the working hours in a day and the working days in a week. However, we consider any time spent by a person while assigned to a plan as _busy_. Then, only if a person is unemployed, or just on standby at a Firm, they are not busy. We compute a person's `busyness` as an approximate proportion of time spent busy over the last `BUSYNESS_AVERAGING_WINDOW` amount of time.
