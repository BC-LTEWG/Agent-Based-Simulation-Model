# Person
Persons are the smallest agents in the society, and have three primary mechanisms, listed below.

## Aging and Retirement
Persons are not instantiated at the moment they are born, but rather at a constant `INITIAL_AGE`, in time steps of hours. We assume that, before this point, they neither consume labor hours (buying goods) nor produce them (work). 
  - For example, if `INITIAL_AGE = 157680`, they effectively ``enter'' the society at 18 years of age.

Since, at the moment, we model society as that portion of society involved in productive work, retiring consists of a person being removed from the society. (This will change when we extend society by implementing public sector services such as retirement income.) On every time step after instantiation, Persons have a probability of `RANDOM_RETIREMENT_CHANCE` to retire before `GUARANTEED_RETIREMENT_AGE`. On attaining `GUARANTEED_RETIREMENT_AGE`, they automatically retire, if they have not already done so. 
  - For example, if `RANDOM_RETIREMENT_CHANCE = 8.3e-7` and `GUARANTEED_RETIREMENT_AGE = 586920`, all persons are guaranteed to retire before 67 years, with the average being around 62 years of age.

## Shopping
Persons do not buy goods continuously; they instead go shopping once every `PERSON_SHOPPING_PERIOD` time steps, with an offset stochastically assigned to each person.

The amount of each good each person will buy depends upon each `Product`'s `mean_consumption_frequency`, multiplied by `PERSON_SHOPPING_PERIOD`. This value is also varied stochastically for each person by `PERSON_FREQUENCY_MULTIPLIER_STDDEV` and varied again for each individual shopping event by `PERSON_SHOPPING_MULTIPLIER_STDDEV`. If a person doesn't have enough labor hour credits to purchase the necessary goods, these quantities are proportionally scaled down futher until the person can afford to purchase them. 

To purchase each good, a person searches sequentially through their list of `ranked_distributors` (a random permutation for each person), until they find a `Distributor` with the item in stock.

## Work
A person's rate of work is determined by the product being produced (specifically, its `required_abilities`), their own `abilities`, and their `health_status`. 

Each of a person's abilities is a `double` representing their skill level in that line of work, with `1.0` representing an average work rate. These values are determined for each person at construction by constants `PERSON_ABILITY_COUNT_MAX` and `PERSON_ABILITY_STDDEV`. For each of a product's `required_abilities`, a person's ability values are averaged, then multiplied by `get_current_productivity()`, to help a firm to decide how suitable that person is to help produce that product. 

For now, `get_current_productivity()` only depends on whether a person is `HEALTHY` or `UNHEALTHY`. We plan to complicate this determination to consider the person's abilities, experience, etc.

