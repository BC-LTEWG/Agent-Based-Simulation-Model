# Person
Persons are the smallest agents in the Society, and have 3 primary mechanisms, listed below.

## Aging and Retirement
Persons are not instantiated at the moment they are born, but rather at a constant `INITIAL_AGE`, in time steps of hours. We assume that before this point, they neither consume labor hours (buying goods) nor produce them (work). 
 - Ex: If `INITIAL_AGE = 157680`, they effectively "enter" the society at 18 years of age.

Retiring consists of a Person being removed from Society. On every timestep after instantiation, Persons have a probability of `RANDOM_RETIREMENT_CHANCE` to retire before `GUARANTEED_RETIREMENT_AGE`. On attaining `GUARANTEED_RETIREMENT_AGE`, they automatically retire. 
 - Ex: If `RANDOM_RETIREMENT_CHANCE = 8.3e-7` and `GUARANTEED_RETIREMENT_AGE = 586920`, all Persons are guaranteed to retire before 67 years, with the average being around 62 years of age.

## Shopping
Persons do not buy goods continuously; they instead go shopping once every `PERSON_SHOPPING_PERIOD` timesteps, with an offset stochastically assigned to each Person.

The amounts of each good each Person will buy is dependent on each `Product`'s `mean_consumption_frequency`, multiplied by `PERSON_SHOPPING_PERIOD`. This value is also varied stochastically for each Person by `PERSON_FREQUENCY_MULTIPLIER_STDDEV` and varied again for each individual shopping event by `PERSON_SHOPPING_MULTIPLIER_STDDEV`. If a Person doesn't have enough labor hours to purchase the necessary goods, these quantities are proportionally scaled down futher until they are able to purchase them. 

To purchase each good, a Person searches sequentially through their list of `ranked_distributors` (a random permutation for each Person), until they find a `Distributor` with the item in stock.

## Work
A Person's rate of work is determined by the Product being produced (specifically, its `required_abilities`), their own `abilities`, and their `health_status`. 

Each of Person's abilities is a `double` representing their skill level in that line of work, with `1.0` representing an average work rate. These values are determined for each Person at construction by constants `PERSON_ABILITY_COUNT_MAX` and `PERSON_ABILITY_STDDEV`. For each of Product's `required_abilities`, a Person's ability values are averaged, then multiplied by `get_current_productivity()`. 

For now, `get_current_productivity()` only depends on whether a Person is `HEALTHY` or `UNHEALTHY`.

